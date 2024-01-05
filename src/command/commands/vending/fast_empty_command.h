//#pragma once
//
//#include "../command_base.h"
//
//namespace commands {
//    class FastEmptyCommand : public command::CommandBase {
//    public:
//        FastEmptyCommand() : command::CommandBase(
//                {"fastempty", "fe"},
//                {"[enable_disable]"},
//                "fast empty the vending machine stock",
//                0
//        ) {}
//
//        void execute(client::Client* client, std::vector<std::string> args) override;
//    };
//}
