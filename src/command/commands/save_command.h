#pragma once

#include "command_base.h"

namespace commands {
class SaveCommand : public command::CommandBase {
public:
    SaveCommand() : command::CommandBase(
            {"save", "save_world"},
            {"file_name"},
            "Dump current world into specified file name by file_name. (need to re-enter if some tile has changed)"
            ) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
