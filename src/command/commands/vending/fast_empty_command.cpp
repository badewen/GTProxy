//#include "fast_empty_command.h"
//
//#include "../../client/client.h"
//#include "../../../module/modules/cheats/fast_vend_module.h"
//
//using namespace commands;
//
//void FastEmptyCommand::execute(client::Client *client, std::vector<std::string> args) {
//    auto mod = (modules::FastVendModule*)client->get_ctx()->ModuleMgr.get_module_by_name("FastVend_Module").get();
//
//    try {
//        if (args.empty()) {
//            mod->enable_fast_empty(true);
//            return;
//        }
//        if (args[0] == "enable") {
//            mod->enable_fast_empty(true);
//        }
//        else {
//            mod->enable_fast_empty(false);
//        }
//    }
//    catch (...) {
//        client->log_to_client("`4Error has occurred");
//    }
//
//}
//
//
