#include "client.h"

#include <chrono>

#include <magic_enum.hpp>
#include <proton/shared/klv.h>

#include "../server/server.h"
#include "../utils/text_parse.h"

using namespace std::chrono_literals;

namespace client {
bool Client::process_incoming_packet(ENetPacket* packet)
{
    packet::eNetMessageType message_type{packet::get_message_type(packet) };
//    std::string message_data{ packet::get_text(packet) };

    if (message_type != packet::NET_MESSAGE_GAME_PACKET) {
        bool forward_packet = true;

        m_ctx->OnIncomingPacket.Invoke(packet, &forward_packet);

        if (!forward_packet) return false;
    }

    switch (message_type) {
        case packet::NET_MESSAGE_SERVER_HELLO: {
            while (this->m_ctx->IsLoginDataSent) { std::this_thread::sleep_for(1ms); }

            utils::TextParse text_parse = utils::TextParse{this->m_ctx->LoginData};

            utils::LoginSpoofData spoof_data = this->m_ctx->LoginSpoofData;

            text_parse.add_key_once("klv|");

            if (Config::get_misc().m_force_update_game_version) {
                text_parse.set("game_version", Config::get_server().m_game_version);
            }
            if (Config::get_misc().m_force_update_protocol) {
                text_parse.set("protocol", Config::get_server().m_protocol);
            }
            //  text_parse.set("platformID", Config::m_server.platformID);
            text_parse.set("mac", spoof_data.Spoofed_mac);
            text_parse.set("rid", spoof_data.Spoofed_rid);
            text_parse.set("wk", spoof_data.Spoofed_wk);
            text_parse.set("hash", spoof_data.Spoofed_device_id_hash);
            text_parse.set("hash2", spoof_data.Spoofed_mac_hash);
            text_parse.set(
                    "klv",
                    proton::generate_klv(
                            text_parse.get<std::uint16_t>("protocol", 1),
                            text_parse.get("game_version", 1),
                            text_parse.get("rid", 1)
                    )
            );

            spdlog::debug(std::string("Received Hello packet, sending ") + (Config::get_misc().m_force_update_game_version ? "modified" : "unmodified") + "login data..");
            send_to_server(player::Peer::build_packet(packet::NET_MESSAGE_GENERIC_TEXT, text_parse.get_all_raw()));

            this->m_ctx->IsLoginDataSent = true;

            return false;
        }
        case packet::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* game_update_packet{packet::get_tank_packet(packet) };
            return process_incoming_raw_packet(game_update_packet);
        }
        default:
            break;
    }
    return true;
}

bool Client::process_incoming_raw_packet(packet::GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != packet::PACKET_CALL_FUNCTION) {
        bool forward_packet = true;

        m_ctx->OnIncomingTankPacket.Invoke(game_update_packet, &forward_packet);

        if (!forward_packet) return false;
    }

    switch (game_update_packet->type) {
        case packet::PACKET_CALL_FUNCTION: {
            std::uint8_t* extended_data{packet::get_extended_data(game_update_packet) };
            if (!extended_data) {
                break;
            }
            VariantList variant_list{};
            variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));
            return process_incoming_variant_list(&variant_list, game_update_packet->net_id);
        }
       case packet::PACKET_SEND_MAP_DATA: {
           std::uint8_t* raw_world_data{ packet::get_extended_data(game_update_packet) };
           if (!raw_world_data) {
               break;
           }
           std::vector<std::uint8_t> raw_world_data_vector{ raw_world_data, raw_world_data + game_update_packet->data_size };

           m_curr_world.set_world_data(raw_world_data_vector);

           return true;
       }
        default:
            break;
    }
    return true;
}
bool Client::process_incoming_variant_list(VariantList *packet, int32_t net_id) {
    std::size_t hash{ utils::hash::fnv1a(packet->Get(0).GetString()) };

    bool forward_packet = true;
    m_ctx->OnIncomingVarlist.Invoke(packet, net_id, &forward_packet);
    if (!forward_packet) return false;

    switch (hash) {
        case "OnSendToServer"_fh: {
            std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(packet->Get(4).GetString()) };
            this->m_ctx->RedirectIp = std::move(tokenize[0]);
            this->m_ctx->RedirectPort = static_cast<enet_uint16>(packet->Get(1).GetINT32());

            spdlog::debug(packet->GetContentsAsDebugString());

            send_to_gt_client(player::Peer::build_variant_packet({
                "OnSendToServer",
                Config::get_host().m_port,
                packet->Get(2).GetINT32(),
                packet->Get(3).GetINT32(),
                fmt::format(
                    "127.0.0.1|{}|{}",
                    tokenize.size() == 2
                        ? ""
                        : tokenize.at(1),
                    tokenize.size() == 2
                        ? tokenize.at(1)
                        : tokenize.at(2)
                ),
                packet->Get(5).GetINT32()
            }, net_id, ENET_PACKET_FLAG_RELIABLE));
            return false;
        }

        case "OnSpawn"_fh: {
            utils::TextParse text_parse{packet->Get(1).GetString() };
            if (text_parse.get("type", 1) == "local") {
                this->m_curr_player.NetID = text_parse.get<int32_t>("netID", 1);

                // Set mods zoom, country flag to JP
                text_parse.set("country", "jp");
                text_parse.set("mstate", 1);

                packet->Get(1).Set(text_parse.get_all_raw());
                send_to_gt_client(
                        player::Peer::build_variant_packet(
                        std::move(*packet),
                        net_id,
                        ENET_PACKET_FLAG_RELIABLE
                    )
                );
                return false;
            }
        }

        case "SetHasGrowID"_fh: {
            this->m_curr_player.PlayerName = packet->Get(2).GetString();
            return true;
        }

        case "OnSetClothing"_fh: {
            send_to_gt_client_delayed(player::Peer::build_variant_packet(*packet, net_id, ENET_PACKET_FLAG_RELIABLE), 250);
            return false;
        }

        case "OnSetRoleSkinsAndIcons"_fh: {
            return true;
        }

        case "OnRequestWorldSelectMenu"_fh: {
            this->m_curr_world.data.clear();
            return true;
        }

        default:
            break;
    }
    return true;
}
}