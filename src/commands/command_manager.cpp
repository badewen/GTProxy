//
// Created by User on 20/10/2023.
//

#include "command_manager.h"
using namespace command;

CommandManager::CommandManager(client::Client *client)
    : m_client { client }
    , m_command_list {}
{

}

void CommandManager::ExecuteCommand(std::string command_alias, std::vector<std::string> args) {
    for (auto& cmd_mod : m_command_list) {
        if (cmd_mod.FindAlias(command_alias))
            return cmd_mod.Execute(m_client, std::move(args));
    }
}

