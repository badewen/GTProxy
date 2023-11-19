#include "sleep_command.h"

#include <thread>

#include "../../client/client.h"
#include "../../peer/peer.h"

using namespace commands;
using namespace std::chrono_literals;

void SleepCommand::execute(client::Client* client, std::vector<std::string> args) {
    client->log_to_client("Sleeping for 10000ms");
    std::this_thread::sleep_for(10000ms);
    client->log_to_client("Done sleeping");
}
