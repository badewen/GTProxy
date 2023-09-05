#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/hash.h"
#include "../utils/text_parse.h"
#include "../eventmanager/event_manager.h"

namespace client {
Client::Client(Config* config, server::Http* http, server::Server* server)
    : enet_wrapper::ENetClient{}
    , m_config{ config }
    , m_http{ http }
    , m_peer{ nullptr }
    , m_server{ server }
    , m_redirect_server{}
{
    event_manager::SetOnReceivePacket([&](ENetPeer* peer, ENetPacket* packet, event_manager::user_data* out) -> void {
        out->block_packet = process_packet(peer, packet);
    });

    event_manager::AddOnReceiveTankPacket([&](ENetPeer* peer, player::GameUpdatePacket* packet,
                                              event_manager::user_data* out) -> void {
        out->block_packet = process_tank_update_packet(peer, packet) || out->block_packet;
    }, "ClientInternal");

    event_manager::AddOnReceiveRawPacket([&](ENetPeer* peer, ENetPacket* packet, event_manager::user_data* out)
                                                 -> void {
        out->block_packet = process_raw_packet(peer, packet) || out->block_packet;
    }, "ClientInternal");

    event_manager::AddOnReceiveVariantlist([&](ENetPeer* peer, VariantList* variant_list, event_manager::user_data* out)
                                                   -> void {
        out->block_packet = process_variant_list(peer, variant_list) || out->block_packet;
    }, "ClientInternal");
}

Client::~Client()
{
    event_manager::RemoveOnReceivePacket();
    event_manager::RemoveOnReceiveRawPacket("ClientInternal");
    event_manager::RemoveOnReceiveVariantlist("ClientInternal");
    event_manager::RemoveOnReceiveTankPacket("ClientInternal");
    delete m_peer.m_gt_server;
}

void Client::start()
{
    if (m_redirect_server.m_host.empty()) {
        utils::TextParse text_parse{ m_http->get_server_data() };
        m_redirect_server.m_host = text_parse.get("server", 1);
        m_redirect_server.m_port = text_parse.get<enet_uint16>("port", 1);
        m_redirect_server.m_using_new_packet = text_parse.get<enet_uint8>("type2", 1);
    }

    if (!create_host(1, m_redirect_server.m_using_new_packet)) {
        spdlog::error("Failed to create ENet client host.");
        return;
    }

    spdlog::info("Connecting to Growtopia server ({}:{}).", m_redirect_server.m_host, m_redirect_server.m_port);

    if (!connect(m_redirect_server.m_host, m_redirect_server.m_port, m_peer.m_client_connect_id)) {
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

    m_peer.m_client_connect_id = peer->connectID;

    m_peer.m_gt_server = new player::Peer{ peer };
    m_server->set_gt_server_peer(m_peer.m_gt_server);
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_peer.m_gt_client) {
        return;
    }

    if (!m_peer.m_gt_client->is_connected()) {
        return;
    }


    if (event_manager::InvokeOnReceivePacket(peer, packet)) {
        spdlog::debug("INCOMING PACKET BLOCKED");
        return;
    }

    m_peer.m_gt_client->send_packet_packet(packet);
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Disconnected from Growtopia server.");

    if (m_peer.m_gt_client && m_peer.m_gt_client->is_connected()) {
        m_peer.m_gt_client->disconnect();
    }

    delete m_peer.m_gt_server;
    m_peer.m_gt_server = nullptr;
    m_server->set_gt_server_peer(nullptr);
}

bool Client::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{player::get_message_type(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        return event_manager::InvokeOnReceiveRawPacket(peer, packet);
    }

    switch (message_type) {
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{player::get_tank_packet(packet) };
            return event_manager::InvokeOnReceiveTankPacket(peer, game_update_packet);
        }
        default:
            break;
    }
    return false;
}

bool Client::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != player::PACKET_STATE && game_update_packet->type != player::PACKET_CALL_FUNCTION &&
        game_update_packet->type != player::PACKET_SEND_MAP_DATA &&
        game_update_packet->type != player::PACKET_SEND_ITEM_DATABASE_DATA
    ) {
        const std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
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

            return event_manager::InvokeOnReceiveVariantlist(peer, &variant_list);
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

    return false;
}

bool Client::process_raw_packet(ENetPeer *peer, ENetPacket *packet) {
    std::string message_data{ player::get_text(packet) };
    player::eNetMessageType message_type{player::get_message_type(packet) };

    utils::TextParse text_parse{ message_data };
    if (!text_parse.empty()) {
        spdlog::info(
                "Incoming MessagePacket:\n{} [{}]:\n{}\n",
                magic_enum::enum_name(message_type),
                message_type,
                fmt::join(text_parse.get_all_array(), "\r\n")
        );
    }

    return false;
}

bool Client::process_variant_list(ENetPeer *peer, VariantList *variant_list) {
    spdlog::info("Incoming VariantList:\n{}", variant_list->GetContentsAsDebugString());

    std::size_t hash{ utils::hash::fnv1a(variant_list->Get(0).GetString()) };
    switch (hash) {
        case "OnSendToServer"_fh: {
            std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list->Get(4).GetString()) };

            m_redirect_server.m_host = std::move(tokenize[0]);
            m_redirect_server.m_port = static_cast<enet_uint16>(variant_list->Get(1).GetINT32());

            m_peer.m_gt_client->send_variant({
                "OnSendToServer",
                m_config->get_host().m_port,
                variant_list->Get(2).GetINT32(),
                variant_list->Get(3).GetINT32(),
                fmt::format(
                        "127.0.0.1|{}|{}",
                        tokenize.size() == 2
                        ? ""
                        : tokenize.at(1),
                        tokenize.size() == 2
                        ? tokenize.at(1)
                        : tokenize.at(2)
                ),
                variant_list->Get(5).GetINT32()
            });
            return true;
        }
        case "OnSpawn"_fh: {
            utils::TextParse text_parse{ variant_list->Get(1).GetString() };
            if (text_parse.get("type", 1) == "local") {
                // Set mods zoom, country flag to JP
                text_parse.set("country", "jp");
                text_parse.set("mstate", 1);

                variant_list->Get(1).Set(text_parse.get_all_raw());

                spdlog::debug(variant_list->GetContentsAsDebugString());

                m_peer.m_gt_client->send_variant(std::move(*variant_list));
                return true;
            }
        }
        default:
            break;
    }
    return false;
}

}
