#pragma once

#include "command_base.h"

namespace commands {
class FastStockCommand : public command::CommandBase {
public:
    FastStockCommand() : command::CommandBase(
            {"faststock", "fs"},
            {"[enable_disable]"},
            "fast stock vending machine",
            0
    ) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
