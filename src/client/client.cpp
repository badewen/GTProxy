#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/hash.h"
#include "../utils/text_parse.h"
#include "../eventmanager/event_manager.h"

namespace client {
Client::Client(server::Server* server)
    : enet_wrapper::ENetClient{}
    , m_proxy_server{ server }
    , m_redirect_server{}
{

}

Client::~Client()
{
}

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

    if (!connect(m_redirect_server.m_host, m_redirect_server.m_port, m_proxy_server->get_gt_client_connect_id())) {
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

    m_gt_server = new player::Peer{ peer };
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_proxy_server->is_gt_client_valid()) {
        return;
    }

    if (!process_packet(peer, packet)) {
        return;
    }

    m_proxy_server->send_to_gt_client(packet);
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Disconnected from Growtopia server.");

    if (m_proxy_server->is_gt_client_valid()) {
        m_proxy_server->get_gt_client_peer()->disconnect();
    }

    delete m_gt_server;
    m_gt_server = nullptr;
}

bool Client::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::message_type_to_string(packet) };
    std::string message_data{ player::get_text(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info(
                "Incoming MessagePacket:\n{} [{}]:\n{}\n",
                magic_enum::enum_name(message_type),
                message_type,
                fmt::join(text_parse.get_all_array(), "\r\n")
            );
        }
    }

    switch (message_type) {
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
            return process_tank_update_packet(peer, game_update_packet);
        }
        default:
            break;
    }
    return true;
}

bool Client::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != player::PACKET_STATE && game_update_packet->type != player::PACKET_CALL_FUNCTION) {
        std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
        std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

        spdlog::info(
            "Incoming TankUpdatePacket:\n [{}]{}{}", 
            game_update_packet->type,
            magic_enum::enum_name(static_cast<player::ePacketType>(game_update_packet->type)),
            extended_data 
                ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector)) 
                : ""
        );
    }

    switch (game_update_packet->type) {
        case player::PACKET_CALL_FUNCTION: {
            std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
            if (!extended_data) {
                break;
            }

            VariantList variant_list{};
            variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));

            spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());

            std::size_t hash{ utils::hash::fnv1a(variant_list.Get(0).GetString()) };
            switch (hash) {
                case "OnSendToServer"_fh: {
                    std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };

                    m_redirect_server.m_host = std::move(tokenize[0]);
                    m_redirect_server.m_port = static_cast<enet_uint16>(variant_list.Get(1).GetINT32());

                    m_proxy_server->send_to_gt_client( player::Peer::build_variant_packet({
                        "OnSendToServer",
                        Config::get_host().m_port,
                        variant_list.Get(2).GetINT32(),
                        variant_list.Get(3).GetINT32(),
                        fmt::format(
                            "127.0.0.1|{}|{}",
                            tokenize.size() == 2
                                ? ""
                                : tokenize.at(1),
                            tokenize.size() == 2
                                ? tokenize.at(1)
                                : tokenize.at(2)
                        ),
                        variant_list.Get(5).GetINT32()
                    }, game_update_packet->net_id, ENET_PACKET_FLAG_RELIABLE));
                    return false;
                }
                case "OnSpawn"_fh: {
                    utils::TextParse text_parse{ variant_list.Get(1).GetString() };
                    if (text_parse.get("type", 1) == "local") {
                        // Set mods zoom, country flag to JP
                        text_parse.set("country", "jp");
                        text_parse.set("mstate", 1);

                        variant_list.Get(1).Set(text_parse.get_all_raw());

                        m_proxy_server->send_to_gt_client( player::Peer::build_variant_packet(
                                std::move(variant_list), game_update_packet->net_id, ENET_PACKET_FLAG_RELIABLE
                        ));
                        return false;
                    }
                }
                default:
                    break;
            }

            break;
        }

        case player::PACKET_SEND_MAP_DATA: {
            std::uint8_t* raw_world_data{ player::get_extended_data(game_update_packet) };
            if (!raw_world_data) {
                break;
            }
            std::vector<std::uint8_t> raw_world_data_vector{ raw_world_data, raw_world_data + game_update_packet->data_size };

            event_manager::InvokeOnWorldEnter(raw_world_data_vector);

        }

        default:
            break;
    }

    return true;
}
}
