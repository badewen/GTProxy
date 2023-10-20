#include "client.h"

#include "magic_enum.hpp"
#include "proton/shared/klv.h"
#include "spdlog/fmt/bin_to_hex.h"

#include "../utils/randutils.hpp"
#include "../utils/random.h"
#include "../utils/text_parse.h"
#include "../server/server.h"

namespace client {
bool Client::process_outgoing_packet(ENetPacket* packet)
{
    packet::eNetMessageType message_type{packet::get_message_type(packet) };
    std::string message_data{packet::get_text(packet) };

    if (message_type != packet::NET_MESSAGE_GAME_PACKET) {
        bool forward_packet = true;

        m_on_outgoing_packet(packet, &forward_packet);

        if (!forward_packet) return false;
    }

    switch (message_type) {
        case packet::NET_MESSAGE_GENERIC_TEXT: {
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
                    send_to_server(player::Peer::build_packet(
                            packet::eNetMessageType::NET_MESSAGE_GAME_MESSAGE,
                            fmt::format("action|join_request\nname|{}\ninvitedWorld|0", world)
                    ));
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
            // this condition wont execute anymore, but better be safe
            else if (message_data.find("requestedName") != std::string::npos) {
                utils::TextParse text_parse{ message_data };

                utils::LoginSpoofData spoof_data = this->m_ctx->LoginSpoofData;

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

//                spdlog::debug("{}", text_parse.get_all_raw());

                send_to_server(player::Peer::build_packet(message_type, text_parse.get_all_raw()));
                return false;
            }
            break;
        }
        case packet::NET_MESSAGE_GAME_MESSAGE: {
            if (message_data.find("action|quit") != std::string::npos && message_data.length() <= 15) {
                m_ctx->GtClientPeer->disconnect();
                m_peer_wrapper->disconnect();
            }

            break;
        }
        case packet::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* game_update_packet{packet::get_tank_packet(packet)  };
            return process_outgoing_raw_packet(game_update_packet);
        }
        default:
            break;
    }

    return true;
}

bool Client::process_outgoing_raw_packet(packet::GameUpdatePacket* game_update_packet)
{
    bool forward_packet = true;
    m_on_outgoing_tank_packet(game_update_packet, &forward_packet);
    if (!forward_packet) return false;

    switch (game_update_packet->type) {
        case packet::PACKET_DISCONNECT:
            m_ctx->GtClientPeer->disconnect_now();
            break;
        default:
            break;
    }

    return true;
}
}