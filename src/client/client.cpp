#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/text_parse.h"
#include "enet/enet.h"

namespace client {
Client::Client(server::Server* server)
    : enet_wrapper::ENetClient{}
    , m_proxy_server { server }
    , m_outgoing_packet_queue {10}
{
}

Client::~Client() = default;

void Client::start(std::shared_ptr<ClientContext> new_ctx)
{
    this->m_ctx = std::move(new_ctx);
    if (!create_host(1, true)) {
        spdlog::error("Failed to create ENet client host.");
        return;
    }

    spdlog::info("Connecting to Growtopia server ({}:{}).", m_ctx->RedirectIp, m_ctx->RedirectPort);

    if (!connect(m_ctx->RedirectIp,
                 m_ctx->RedirectPort,
                 m_proxy_server->get_gt_client_connect_id(this))) {
        spdlog::error("Failed connect to Growtopia server.");
        return;
    }

    start_service();
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info("Connected to Growtopia server.");
}

void Client::on_service_loop()
{
    ENetPacket* packet = nullptr;

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
    while (m_outgoing_packet_queue.pop()) {}

    if (m_proxy_server->is_gt_client_valid(this)) {
        m_ctx->GtClientPeer->disconnect();
    }

    m_ctx->GtClientPeer = nullptr;
    m_peer_wrapper = nullptr;
    m_ctx = nullptr;
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
                spdlog::info("Incoming VariantList with netid {}:\n{}", -1,
                                variant_list.GetContentsAsDebugString());
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

    m_ctx->GtClientPeer->send_packet_packet(packet);
}
}
