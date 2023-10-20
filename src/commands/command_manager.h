#pragma once

namespace client {
class Client;
}

#include "command_base.h"

namespace command {
class CommandManager {
public:
    CommandManager(client::Client *client);

    void ExecuteCommand(std::string command_alias, std::vector<std::string> args);

private:
    client::Client *m_client;
    std::vector<command::CommandBase> m_command_list;
};
}