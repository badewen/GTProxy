#include "command_manager.h"

#include "warp.h"
#include "save.h"

using namespace command;

CommandManager::CommandManager(client::Client *client)
    : m_client { client }
    , m_command_list {}
{
    m_command_list.push_back(new commands::Warp());
    m_command_list.push_back(new commands::Save());
}

CommandManager::~CommandManager() {
    for (auto& cmd_mod : m_command_list) {
        delete cmd_mod;
    }
    m_command_list.clear();
}

void CommandManager::ExecuteCommand(const std::string& command_alias, std::vector<std::string> args) {
    for (auto& cmd_mod : m_command_list) {
        if (cmd_mod->FindAlias(command_alias))
            return cmd_mod->Execute(m_client, std::move(args));
    }
}

