#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/text_parse.h"
#include "enet/enet.h"
#include "../utils/timer.h"

using namespace std::chrono_literals;

namespace client {
Client::Client(server::Server* server, std::shared_ptr<BS::thread_pool> thread_pool)
    : enet_wrapper::ENetClient{}
    , m_proxy_server { server }
    , m_primary_packet_queue {10}
    , m_secondary_packet_queue {10}
    , m_curr_world {}
    , m_curr_player {}
    , m_thread_pool { thread_pool }
    , m_command_manager { this, thread_pool }
{
    spdlog::debug("NEW CLIENT CLASS IS CREATED");
}

Client::~Client() = default;

void Client::start(std::shared_ptr<ClientContext> new_ctx)
{
    this->m_ctx = std::move(new_ctx);
    if (!m_host) {
        if (!create_host(1, m_ctx->UseModifiedENet)) {
            spdlog::error("Failed to create ENet client host.");
            return;
        }
    }

    spdlog::info("Connecting to Growtopia server ({}:{}).", m_ctx->RedirectIp, m_ctx->RedirectPort);

    if (!connect(m_ctx->RedirectIp,
                 m_ctx->RedirectPort,
                 m_proxy_server->get_gt_client_connect_id(this))) {
        spdlog::error("Failed connect to Growtopia server.");
        return;
    }

    run_service();
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info("Connected to Growtopia server.");

    m_ctx->ModuleMgr.update_curr_client(this);
}

void Client::on_service_loop()
{
    PacketInfo packet_info;
    bool forward_packet = true;

    while (is_valid() && m_primary_packet_queue.try_dequeue(packet_info) && m_ctx) {

        if (!packet_info.Delay.IsDone()) {
            m_secondary_packet_queue.enqueue(packet_info);
            continue;
        }

        if (packet_info.ShouldProcess) {
            forward_packet = packet_info.IsOutgoing ? process_outgoing_packet(packet_info.Packet)
                                                    : process_incoming_packet(packet_info.Packet);
        }

        if (!forward_packet) continue;

        if (packet_info.IsOutgoing) {
            send_to_server(packet_info.Packet);
        }
        else {
            send_to_gt_client(packet_info.Packet);
        }
    }
    std::swap(m_primary_packet_queue, m_secondary_packet_queue);
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_proxy_server->is_gt_client_valid(this)) {
        return;
    }

    queue_packet(packet, false);
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Disconnected from growtopia server.");

    // empty out
    PacketInfo temp;
    while (m_primary_packet_queue.try_dequeue(temp)) { enet_packet_destroy(temp.Packet); }

    if (m_proxy_server->is_gt_client_valid(this)) {
        m_ctx->GtClientPeer->disconnect();
    }

    this->m_curr_player.NetID = 0;
    this->m_curr_player.PlayerName = "";

    this->m_curr_world.data.clear();

    m_ctx->IsConnected = false;
    m_ctx->GtClientPeer = nullptr;
    m_ctx->ModuleMgr.update_curr_client(nullptr);
    m_peer_wrapper = nullptr;
    m_ctx = nullptr;

    m_running.store(false);
}

void Client::send_to_server(ENetPacket *packet) {
    packet::eNetMessageType message_type{packet::get_message_type(packet) };

    switch (message_type) {
        case packet::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* game_update_packet = packet::get_tank_packet(packet);
            std::uint8_t* extended_data{packet::get_extended_data(game_update_packet) };
            std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

            spdlog::info(
                    "Outgoing TankUpdatePacket:\n [{}]{}{}",
                    game_update_packet->type,
                    magic_enum::enum_name(static_cast<packet::ePacketType>(game_update_packet->type)),
                    extended_data
                    ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector))
                    : ""
            );
            break;
        }

        default: {
            std::string message_data{packet::get_text(packet) };
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

void Client::send_to_gt_client(ENetPacket *packet, bool destroy_packet) {
    packet::eNetMessageType message_type{packet::get_message_type(packet) };

    if (message_type != packet::NET_MESSAGE_GAME_PACKET) {
        std::string message_data{packet::get_text(packet) };
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
        packet::GameUpdatePacket* game_update_packet = packet::get_tank_packet(packet);

        std::uint8_t* extended_data{packet::get_extended_data(game_update_packet) };
        std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

        switch (game_update_packet->type) {

            case packet::ePacketType::PACKET_CALL_FUNCTION: {
                VariantList variant_list{};
                variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));
                spdlog::info("Incoming VariantList with netid {}:\n{}", game_update_packet->net_id,
                                variant_list.GetContentsAsDebugString());
                break;
            }

            default: {
                spdlog::info(
                        "Incoming TankUpdatePacket:\n [{}]{}{}",
                        game_update_packet->type,
                        magic_enum::enum_name(static_cast<packet::ePacketType>(game_update_packet->type)),
                        extended_data
                        ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector))
                        : ""
                );
                break;
            }
        }
    }

    m_ctx->GtClientPeer->send_packet_packet(packet, destroy_packet);
}

void Client::log_to_client(const std::string &message) {
    if (!is_valid() && is_ctx_empty()) return;

    send_to_gt_client( player::Peer::build_variant_packet({
            "OnConsoleMessage",
            fmt::format("[`2GTPROXY`o] {}", message)
        }, -1, ENET_PACKET_FLAG_RELIABLE)
    );
}

}
