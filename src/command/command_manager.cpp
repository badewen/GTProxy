#include "command_manager.h"

#include <utility>

#include "commands/warp_command.h"
#include "commands/save_command.h"
#include "commands/sleep_command.h"
#include "commands/fast_drop_command.h"
#include "commands/fast_buy_command.h"
#include "commands/fast_empty_command.h"
#include "commands/fast_stock_command.h"
#include "commands/auto_fish_command.h"

#include "../client/client.h"

using namespace command;


CommandManager::CommandManager(client::Client *client, std::shared_ptr<BS::thread_pool> thread_pool)
    : m_client { client }
    , m_command_list {}
    , m_thread_pool { std::move(thread_pool) }
{
    m_command_list.push_back(std::make_unique<commands::WarpCommand>());
    m_command_list.push_back(std::make_unique<commands::SaveCommand>());
    m_command_list.push_back(std::make_unique<commands::SleepCommand>());
    m_command_list.push_back(std::make_unique<commands::FastDropCommand>());
    m_command_list.push_back(std::make_unique<commands::FastBuyCommand>());
    m_command_list.push_back(std::make_unique<commands::FastEmptyCommand>());
    m_command_list.push_back(std::make_unique<commands::FastStockCommand>());
    m_command_list.push_back(std::make_unique<commands::AutoFishCommand>());
}

CommandManager::~CommandManager() = default;

void CommandManager::execute_command(const std::string& command_alias, std::vector<std::string> args) {
    for (auto& cmd_mod : m_command_list) {
        if (cmd_mod->find_alias(command_alias)) {
            if (args.size() < cmd_mod->get_minimum_args_count() && cmd_mod->get_minimum_args_count()) {
                m_client->log_to_client(
                         fmt::format("`4Wrong number of argument. required {} argument minimum, but {} is provided",
                                    cmd_mod->get_minimum_args_count(),
                                    args.size())
                );
                m_client->log_to_client(
                        fmt::format("Usage : `#{}", cmd_mod->get_usage_hint())
                );
                return;
            }
            if (cmd_mod->IsThreaded) {
                m_thread_pool->push_task(&CommandBase::execute, cmd_mod.get(), m_client, args);
            }
            else {
                cmd_mod->execute(m_client, std::cref(args));
            }
        }
    }
}

