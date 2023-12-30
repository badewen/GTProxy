#include "auto_fish_command.h"

#include "../../client/client.h"
#include "../../module/modules/cheats/auto_fish_module.h"

using namespace commands;

void AutoFishCommand::execute(client::Client *client, std::vector<std::string> args) {

    try {
        if (args.empty()) {
            client->get_ctx()->ModuleMgr.disable_module("AutoFish_Module");
            return;
        }
        if (args[0] == "enable") {
            client->get_ctx()->ModuleMgr.enable_module("AutoFish_Module");
        }
        else {
            client->get_ctx()->ModuleMgr.disable_module("AutoFish_Module");
        }
    }
    catch (...) {
        client->log_to_client("`4Error has occurred");
    }

}


