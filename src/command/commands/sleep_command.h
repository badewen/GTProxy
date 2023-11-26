#pragma once

#include "command_base.h"


// debugging command

namespace commands {
class SleepCommand : public command::CommandBase {
public:
    SleepCommand() : command::CommandBase{
        {"internal_sleep"},
        {},
        "Debug command for testing threaded command",
        0,
        true
    } {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
