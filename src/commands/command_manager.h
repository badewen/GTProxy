#pragma once

#include "command_base.h"

namespace command {
class CommandManager {
public:
    explicit CommandManager(client::Client *client);
    ~CommandManager();

    void ExecuteCommand(const std::string& command_alias, std::vector<std::string> args);

private:
    client::Client *m_client;
    std::vector<command::CommandBase*> m_command_list;
};
}