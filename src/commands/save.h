#pragma once

#include "command_base.h"

namespace commands {
class Save : public command::CommandBase {
public:
    Save() : command::CommandBase( {"save", "save_world"}, {"file_name"} ) {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
