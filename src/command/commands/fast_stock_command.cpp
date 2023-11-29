#include "fast_stock_command.h"

#include "../../client/client.h"
#include "../../module/modules/fast_vend_module.h"

using namespace commands;

void FastStockCommand::execute(client::Client *client, std::vector<std::string> args) {
    auto mod = (modules::FastVendModule*)client->get_ctx()->ModuleMgr.get_module_by_name("FastVend_Module").get();

    try {
        if (args.size() == 0) {
            mod->enable_fast_stock(true);
            return;
        }
        if (args[0] == "enable") {
            mod->enable_fast_stock(true);
        }
        else {
            mod->enable_fast_stock(false);
        }
    }
    catch (...) {
        client->log_to_client("`4Error has occurred");
    }

}


