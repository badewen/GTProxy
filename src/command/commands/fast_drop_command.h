#pragma once

#include "command_base.h"

namespace commands {
class FastDropCommand : public command::CommandBase {
public:
    //
    FastDropCommand() : command::CommandBase(
            {"fastdrop", "fd"},
            {"amount", "[enable_disable]"},
            "Skip drop dialog.\n if \"enable_disable\" is not specified, it will be defaulted to enable",
            false, true) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
