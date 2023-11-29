#pragma once

#include "command_base.h"

namespace commands {
class FastBuyCommand : public command::CommandBase {
public:
    FastBuyCommand() : command::CommandBase(
            {"fastbuy", "fb"},
            {"amount", "[enable_disable]"},
            "fast buy for vending machine.",
            1
    ) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
