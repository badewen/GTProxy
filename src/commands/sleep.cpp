#include "sleep.h"

#include <thread>

#include "../client/client.h"

using namespace commands;
using namespace std::chrono_literals;

void Sleep::execute(client::Client* client, std::vector<std::string> args) {
    client->send_to_gt_client( player::Peer::build_variant_packet (
                    {
                        "OnConsoleMessage",
                        fmt::format("[GTPROXY] Sleeping for {} ms", 10000)
                    }, -1, ENET_PACKET_FLAG_RELIABLE )
            );
    std::this_thread::sleep_for(10000ms);
    client->send_to_gt_client( player::Peer::build_variant_packet (
            {
                "OnConsoleMessage",
                "[GTPROXY] Sleep completed."
            }, -1, ENET_PACKET_FLAG_RELIABLE )
    );
}
