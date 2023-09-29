#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/text_parse.h"
#include "enet/enet.h"

namespace client {
Client::Client(server::Server* server)
    : enet_wrapper::ENetClient{}
    , m_redirect_server{}
    , m_proxy_server { server }
    , m_gt_client {}
    , m_outgoing_packet_queue {10}
    , m_send_login_data {false}
    , m_redirected {false}
{
}

Client::~Client() = default;

void Client::start()
{
    if (m_redirect_server.m_host.empty()) {
        utils::TextParse text_parse{ server::Http::get_server_data() };
        m_redirect_server.m_host = text_parse.get("server", 1);
        m_redirect_server.m_port = text_parse.get<enet_uint16>("port", 1);
        m_redirect_server.m_using_new_packet = text_parse.get<enet_uint8>("type2", 1);
    }

    if (!create_host(1, m_redirect_server.m_using_new_packet)) {
        spdlog::error("Failed to create ENet client host.");
        return;
    }

    spdlog::info("Connecting to Growtopia server ({}:{}).", m_redirect_server.m_host, m_redirect_server.m_port);

    if (!connect(m_redirect_server.m_host,
                 m_redirect_server.m_port,
                 m_proxy_server->get_gt_client_connect_id(this))) {
        spdlog::error("Failed connect to Growtopia server.");
        return;
    }

    m_redirect_server.m_host.clear();
    m_redirect_server.m_port = 16999;

    start_service();
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info("Connected to Growtopia server.");

    if (is_redirected()) {
        m_redirected = false;
        return;
    }

    m_gt_client = m_proxy_server->get_peer_by_client(this);

}

void Client::on_service_loop()
{
    ENetPacket* packet = nullptr;

    if (is_redirected() && !is_valid()) {
        connect(m_redirect_server.m_host, m_redirect_server.m_port, m_proxy_server->get_gt_client_connect_id(this));
        return;
    }

    if (is_redirected()) {
//        player::GameUpdatePacket pckt {};
//        pckt.type = player::PACKET_DISCONNECT;
//
//        send_to_server(player::Peer::build_raw_packet(&pckt));
        m_peer_wrapper->disconnect();
        return;
    }

    if (m_send_login_data && !m_login_data.empty()) {
        utils::TextParse text_parse {m_login_data};

        text_parse.add_key_once("user");
        text_parse.add_key_once("token");
        text_parse.add_key_once("UUIDToken");
        text_parse.add_key_once("doorID");

        text_parse.set("user", m_redirect_server.m_user);
        text_parse.set("token", m_redirect_server.m_token);
        text_parse.set("UUIDToken", m_redirect_server.m_uuid_token);
        text_parse.set("doorID", m_redirect_server.m_door_id);

        std::string login_data = text_parse.get_all_raw();

        send_to_server(player::Peer::build_packet(
                player::eNetMessageType::NET_MESSAGE_GENERIC_TEXT,
                std::move(login_data)
            )
        );
        m_send_login_data = false;
    }

    while (is_valid() && m_outgoing_packet_queue.try_dequeue(packet)) {
        if (!process_outgoing_packet(packet)) {
            continue;
        }
        send_to_server(packet);
    }
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_proxy_server->is_gt_client_valid(this)) {
        return;
    }

    if (!process_incoming_packet(packet)) {
        return;
    }

    send_to_gt_client(packet);
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Disconnected from growtopia server.");

    if (is_redirected()) {
        return;
    }

    if (m_proxy_server->is_gt_client_valid(this)) {
        m_gt_client->disconnect();
    }

    m_gt_client = nullptr;

    while (m_outgoing_packet_queue.pop()) {}
    if (m_peer_wrapper) delete m_peer_wrapper;
    m_peer_wrapper = nullptr;
}

void Client::send_to_server(ENetPacket *packet) {
    player::eNetMessageType message_type{ player::get_message_type(packet) };

    switch (message_type) {
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet = player::get_tank_packet(packet);
            std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
            std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

            spdlog::info(
                    "Outgoing TankUpdatePacket:\n [{}]{}{}",
                    game_update_packet->type,
                    magic_enum::enum_name(static_cast<player::ePacketType>(game_update_packet->type)),
                    extended_data
                    ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector))
                    : ""
            );
            break;
        }

        default: {
            std::string message_data{ player::get_text(packet) };
            utils::TextParse text_parse{ message_data };

            if (!text_parse.empty()) {
                spdlog::info(
                        "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                        magic_enum::enum_name(message_type),
                        message_type,
                        fmt::join(text_parse.get_all_array(), "\r\n")
                );
            }
            break;
        }
    }

    m_peer_wrapper->send_packet_packet(packet);
}

void Client::send_to_gt_client(ENetPacket *packet) {
    player::eNetMessageType message_type{ player::get_message_type(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        std::string message_data{ player::get_text(packet) };
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info(
                    "Incoming MessagePacket:\n{} [{}]:\n{}\n",
                    magic_enum::enum_name(message_type),
                    message_type,
                    fmt::join(text_parse.get_all_array(), "\r\n"));
        }
    }
    else {
        player::GameUpdatePacket* game_update_packet = player::get_tank_packet(packet);

        std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
        std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

        switch (game_update_packet->type) {

            case player::ePacketType::PACKET_CALL_FUNCTION: {
                VariantList variant_list{};
                variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));
                spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());
                break;
            }

            default: {
                spdlog::info(
                        "Incoming TankUpdatePacket:\n [{}]{}{}",
                        game_update_packet->type,
                        magic_enum::enum_name(static_cast<player::ePacketType>(game_update_packet->type)),
                        extended_data
                        ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector))
                        : ""
                );
                break;
            }
        }
    }

    m_gt_client->send_packet_packet(packet);
}
}
