#include "auto_fish_command.h"

#include "../../client/client.h"
#include "../../module/modules/auto_fish_module.h"

using namespace commands;

void AutoFishCommand::execute(client::Client *client, std::vector<std::string> args) {
    auto mod = (modules::AutoFishModule*)client->get_ctx()->ModuleMgr.get_module_by_name("AutoFish_Module").get();

    try {
        if (args.empty()) {
            mod->enable();
            return;
        }
        if (args[0] == "enable") {
            mod->enable();
        }
        else {
            mod->disable();
        }
    }
    catch (...) {
        client->log_to_client("`4Error has occurred");
    }

}


