#pragma once

#include "command_base.h"

namespace commands {
class Warp : public command::CommandBase {
public:
    Warp() : command::CommandBase( {"warp"} ) {}

    void Execute(client::Client* client, std::vector<std::string> args) override;
};
}
