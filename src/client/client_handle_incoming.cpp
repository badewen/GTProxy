#include "client.h"

#include <magic_enum.hpp>
#include <proton/shared/klv.h>

#include "../server/server.h"
#include "../utils/text_parse.h"

namespace client {
bool Client::process_incoming_packet(ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::get_message_type(packet) };
//    std::string message_data{ player::get_text(packet) };

    switch (message_type) {
        case player::NET_MESSAGE_SERVER_HELLO: {
            utils::LoginSpoofData spoof_data = this->m_ctx->LoginSpoofData;
            utils::TextParse text_parse = utils::TextParse { this->m_ctx->LoginData };

            text_parse.add_key_once("klv|");

            text_parse.set("game_version", Config::get_server().m_game_version);
            text_parse.set("protocol", Config::get_server().m_protocol);
            // text_parse.set("platformID", Config::m_server.platformID);
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

            // have to fix this login packet, smh their budget for growtopia is 1 tortilla
//            if (text_parse.get("doorID", 1).empty() && !text_parse.get("token", 1).empty() && text_parse.get("reconnect", 1).empty()) {
//                text_parse.add("doorID", "0");
//            }

            spdlog::debug("Received Hello packet, sending modified login data..");
            send_to_server(player::Peer::build_packet(player::NET_MESSAGE_GENERIC_TEXT, text_parse.get_all_raw()));

            return false;
        }
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{ player::get_tank_packet(packet) };
            return process_incoming_raw_packet(game_update_packet);
        }
        default:
            break;
    }
    return true;
}

bool Client::process_incoming_raw_packet(player::GameUpdatePacket* game_update_packet)
{
    switch (game_update_packet->type) {
        case player::PACKET_CALL_FUNCTION: {
            std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
            if (!extended_data) {
                break;
            }
            VariantList variant_list{};
            variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));
            return process_incoming_variant_list(&variant_list, game_update_packet->net_id);
        }
//       case player::PACKET_SEND_MAP_DATA: {
//           std::uint8_t* raw_world_data{ player::get_extended_data(game_update_packet) };
//           if (!raw_world_data) {
//               break;
//           }
//           std::vector<std::uint8_t> raw_world_data_vector{ raw_world_data, raw_world_data + game_update_packet->data_size };
//
//           event_manager::InvokeOnWorldEnter(raw_world_data_vector);
//
//       }
        default:
            break;
    }
    return true;
}
bool Client::process_incoming_variant_list(VariantList *packet, int32_t net_id) {
    std::size_t hash{ utils::hash::fnv1a(packet->Get(0).GetString()) };

    switch (hash) {
        case "OnSendToServer"_fh: {
            std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(packet->Get(4).GetString()) };
            this->m_ctx->RedirectIp = std::move(tokenize[0]);
            this->m_ctx->RedirectPort = static_cast<enet_uint16>(packet->Get(1).GetINT32());

            spdlog::debug(packet->GetContentsAsDebugString());

            send_to_gt_client( player::Peer::build_variant_packet({
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
        default:
            break;
    }
    return true;
}
}