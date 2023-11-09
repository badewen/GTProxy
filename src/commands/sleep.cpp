#include "sleep.h"

#include <thread>

#include "../client/client.h"

using namespace commands;
using namespace std::chrono_literals;

void Sleep::execute(client::Client* client, std::vector<std::string> args) {
    client->log_to_client("Sleeping for 10000ms");
    std::this_thread::sleep_for(10000ms);
    client->log_to_client("Done sleeping");
}
