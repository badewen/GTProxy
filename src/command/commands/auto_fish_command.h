#pragma once

#include "command_base.h"

namespace commands {
class AutoFishCommand : public command::CommandBase {
public:
    AutoFishCommand() : command::CommandBase(
            {"autofish", "af"},
            {"[enable_disable]"},
            "auto fish",
            0
    ) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
