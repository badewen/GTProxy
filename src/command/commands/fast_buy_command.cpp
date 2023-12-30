#include "fast_buy_command.h"

#include "../../client/client.h"
#include "../../module/modules/cheats/fast_vend_module.h"

using namespace commands;

void FastBuyCommand::execute(client::Client *client, std::vector<std::string> args) {
    auto mod = (modules::FastVendModule*)client->get_ctx()->ModuleMgr.get_module_by_name("FastVend_Module").get();

    try {
        mod->set_buy_amount(std::stoi(args.at(0)));

        if (args.size() > 1 && args[1] == "enable" || args.size() == 1) {
            mod->enable_fast_buy(true);
        }
        else {
            mod->enable_fast_buy(false);
        }
    }
    catch (...) {
        client->log_to_client("`4Wrong argument type is passed");
    }

}

