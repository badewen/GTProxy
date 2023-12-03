#include "warp_command.h"

#include "../../client/client.h"

using namespace commands;

void WarpCommand::execute(client::Client* client, std::vector<std::string> args) {
    client->send_to_server(player::Peer::build_packet(
            packet::PacketType::NET_MESSAGE_GAME_MESSAGE,
            fmt::format("action|join_request\nname|{}\ninvitedWorld|0", args[0])
    ));
}