#include "command_manager.h"

#include <utility>

#include "warp.h"
#include "save.h"
#include "sleep.h"

#include "../client/client.h"

using namespace command;


CommandManager::CommandManager(client::Client *client, std::shared_ptr<BS::thread_pool> thread_pool)
    : m_client { client }
    , m_command_list {}
    , m_thread_pool { std::move(thread_pool) }
{
    m_command_list.push_back(new commands::Warp());
    m_command_list.push_back(new commands::Save());
    m_command_list.push_back(new commands::Sleep());
}

CommandManager::~CommandManager() {
    for (auto& cmd_mod : m_command_list) {
        delete cmd_mod;
    }
    m_command_list.clear();
}

void CommandManager::execute_command(const std::string& command_alias, std::vector<std::string> args) {
    for (auto& cmd_mod : m_command_list) {
        if (cmd_mod->find_alias(command_alias)) {

            if (args.size() < cmd_mod->get_args_count() && !cmd_mod->is_var_arg()) {
                m_client->log_to_client(
                        fmt::format("`4Wrong number of argument. {} is required but {} was provided.",
                                    cmd_mod->get_args_count(),
                                    args.size())
                );
                m_client->log_to_client(
                        fmt::format("Usage : `#{}", cmd_mod->get_usage_hint())
                );
                return;
            }

            if (cmd_mod->IsThreaded) {
                m_thread_pool->push_task(&CommandBase::execute, cmd_mod, m_client, args);
            }
            else {
                cmd_mod->execute(m_client, std::cref(args));
            }
        }
    }
}

