#pragma once


#include <memory>
#include <threadpool/include/BS_thread_pool.hpp>

#include "command_base.h"

namespace command {
class CommandManager {
public:
    explicit CommandManager(client::Client *client, std::shared_ptr<BS::thread_pool> thread_pool);
    ~CommandManager();

    void execute_command(const std::string& command_alias, std::vector<std::string> args);

private:
    client::Client *m_client;
    std::shared_ptr<BS::thread_pool> m_thread_pool;
    std::vector<command::CommandBase*> m_command_list;
};
}