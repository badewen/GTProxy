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
    packet::PacketType message_type{packet::get_message_type(packet) };
    std::string message_data{packet::get_text(packet) };

    if (message_type != packet::PacketType::NET_MESSAGE_GAME_PACKET) {
        bool forward_packet = true;

        m_ctx->OnOutgoingPacket.Invoke(packet, &forward_packet);

        if (!forward_packet) return false;
    }

    switch (message_type) {
        case packet::PacketType::NET_MESSAGE_GENERIC_TEXT: {
            if (message_data.find("action|input") != std::string::npos) {
                utils::TextParse text_parse{ message_data };
                if (text_parse.get("text", 1).empty()) {
                    break;
                }

                std::vector<std::string> token{ utils::TextParse::string_tokenize( message_data.substr(
                        message_data.find("text|") + 5,
                        message_data.length() - message_data.find("text|") - 1
                ), " " )};

                if (token[0].starts_with(Config::get_command().m_prefix)) {
                    m_command_manager.execute_command(token[0].substr(Config::get_command().m_prefix.length()),
                                                      token.size() == 1 ? std::vector<std::string>{}
                                                                        : std::vector<std::string>{std::next(token.begin()), token.end()}
                    );
                    return false;
                }
            }
            break;
        }
        case packet::PacketType::NET_MESSAGE_GAME_MESSAGE: {
            if (message_data.find("action|quit") != std::string::npos && message_data.length() <= 15) {
                m_ctx->GtClientPeer->disconnect();
                m_peer_wrapper->disconnect();
            }

            break;
        }
        case packet::PacketType::NET_MESSAGE_GAME_PACKET: {
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
    m_ctx->OnOutgoingTankPacket.Invoke(game_update_packet, &forward_packet);
    if (!forward_packet) return false;

    switch (game_update_packet->type) {
        case packet::TankPacketType::PACKET_DISCONNECT: {
            m_ctx->IsConnected = false;
            m_peer_wrapper->disconnect_now();
            m_ctx->GtClientPeer->disconnect_now();
            break;
        }
        default:
            break;
    }

    return true;
}
}