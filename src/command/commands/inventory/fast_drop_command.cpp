//#include "fast_drop_command.h"
//
//#include "../../client/client.h"
//#include "../../../module/modules/cheats/fast_drop_module.h"
//
//using namespace commands;
//
//void FastDropCommand::execute(client::Client *client, std::vector<std::string> args) {
//    auto mod = (modules::FastDropModule*)client->get_ctx()->ModuleMgr.get_module_by_name("FastDrop_Module").get();
//
//    try {
//        if (args[0] == "all") {
//            mod->set_drop_all(true);
//        }
//        else {
//            mod->set_drop_amount(std::stoi(args[0]));
//        }
//
//        if (args.size() > 1 && args[1] == "enable" || args.size() == 1) {
//            mod->enable();
//        }
//        else {
//            mod->set_drop_all(false);
//            mod->disable();
//        }
//    }
//    catch (...) {
//        client->log_to_client("`4Wrong argument type is passed");
//    }
//
//}
//
