#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <klv.h>

#include "server.h"
#include "../client/client.h"
#include "../utils/hash.h"
#include "../utils/random.h"
#include "../utils/text_parse.h"

namespace server {
Server::Server()
    : enet_wrapper::ENetServer{}
    , m_peer {}
{
    m_gt_server_client = new client::Client{ this };
}

Server::~Server()
{
    delete m_gt_server_client;
    delete m_peer.m_gt_server_client;
}

bool Server::start()
{
    if (!create_host(Config::get_host().m_port, 1, 1)) {
        spdlog::error("Failed to create ENet server host.");
        return false;
    }

    start_service();
    spdlog::info("ENet server listening on port {}.", Config::get_host().m_port);
    return true;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info("New client connected to proxy server.");

    m_gt_client = new player::Peer{ peer };
    m_gt_server_client->start();
    m_peer.m_gt_server_client = new player::Peer{ m_gt_server_client->m_peer };
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!is_gt_server_client_valid()) {
        return;
    }
    if (!process_packet(peer, packet)) {
        return;
    }

    m_peer.m_gt_server_client->send_packet_packet(packet);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server.");

    if (m_peer.m_gt_server_client && m_peer.m_gt_server_client->is_connected()) {
        m_peer.m_gt_server_client->disconnect();
    }

    delete m_peer.m_gt_server_client;
    delete m_gt_client;
    m_gt_client = nullptr;
    m_peer.m_gt_server_client = nullptr;
}

bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::get_message_type(packet) };
    std::string message_data{ player::get_text(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info(
                "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                magic_enum::enum_name(message_type),
                message_type,
                fmt::join(text_parse.get_all_array(), "\r\n")
            );
        }
    }

    switch (message_type) {
        case player::NET_MESSAGE_GENERIC_TEXT: {
            if (message_data.find("action|input") != std::string::npos) {
                utils::TextParse text_parse{ message_data };
                if (text_parse.get("text", 1).empty()) {
                    break;
                }

                std::vector<std::string> token{ utils::TextParse::string_tokenize( message_data.substr(
                    message_data.find("text|") + 5,
                    message_data.length() - message_data.find("text|") - 1
                ), " " )};

                if (token[0] == Config::get_command().m_prefix + "warp") {
                    std::string world{token[1]};
                    m_peer.m_gt_server_client->send_packet(
                        player::eNetMessageType::NET_MESSAGE_GAME_MESSAGE,
                        fmt::format("action|join_request\nname|{}\ninvitedWorld|0", world)
                    );
                    return false;
                }
//                else if (token[0] == Config::get_command().m_prefix + "save") {
//                    std::string file_name{token[1]};
//
//                    FILE* f = NULL;
//
//                    fopen_s(&f, file_name.c_str(), "wb");
//                    auto w_data = PlayerStateHandler::GetPlayerState().CurrentWorld.data;
//                    fwrite(w_data.data(), 1, w_data.size(), f);
//
//                    fclose(f);
//
//                    return false;
//                }
            }
            else if (message_data.find("requestedName") != std::string::npos) {
                static randutils::pcg_rng gen{ utils::random::get_generator_local() };
                static std::string mac{ utils::random::generate_mac(gen) };
                static std::int32_t mac_hash{ utils::hash::proton(fmt::format("{}RT", mac).c_str()) };
                static std::string rid{ utils::random::generate_hex(gen, 32, true) };
                static std::string wk{ utils::random::generate_hex(gen, 32, true) };
                static std::string device_id{ utils::random::generate_hex(gen, 16, true) };
                static std::int32_t device_id_hash{ utils::hash::proton(fmt::format("{}RT", device_id).c_str()) };

                utils::TextParse text_parse{ message_data };

                text_parse.add_key_once("klv|");

                text_parse.set("game_version", Config::get_server().m_game_version);
                text_parse.set("protocol", Config::get_server().m_protocol);
                // text_parse.set("platformID", Config::m_server.platformID);
                text_parse.set("mac", mac);
                text_parse.set("rid", rid);
                text_parse.set("wk", wk);
                text_parse.set("hash", device_id_hash);
                text_parse.set("hash2", mac_hash);
                text_parse.set(
                    "klv",
                    proton::generate_klv(
                        text_parse.get<std::uint16_t>("protocol", 1),
                        text_parse.get("game_version", 1),
                        text_parse.get("rid", 1)
                    )
                );

                spdlog::debug("{}", text_parse.get_all_raw());
                m_peer.m_gt_server_client->send_packet(message_type, text_parse.get_all_raw());
                return false;
            }

            break;
        }
        case player::NET_MESSAGE_GAME_MESSAGE: {
            if (message_data.find("action|quit") != std::string::npos && message_data.length() <= 15) {
                m_gt_client->disconnect();
            }

            break;
        }
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{ player::get_tank_packet(packet)  };
            return process_tank_update_packet(peer, game_update_packet);
        }
        default:
            break;
    }

    return true;
}

bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet) const
{
    if (game_update_packet->type != player::PACKET_CALL_FUNCTION) {
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
    }

    switch (game_update_packet->type) {
        case player::PACKET_DISCONNECT:
            m_gt_client->disconnect_now();
            break;
        default:
            break;
    }

    return true;
}
}
