//#pragma once
//
//
//#include <memory>
//#include <threadpool/include/BS_thread_pool.hpp>
//
//#include "commands/command_base.h"
//
//namespace server {
//class Server
//}
//
//namespace command {
//class CommandManager {
//public:
//    explicit CommandManager(server::Server *proxy_server, std::shared_ptr<BS::thread_pool> thread_pool);
//    ~CommandManager();
//
//    void execute_command(const std::string& command_alias, std::vector<std::string> args);
//
//private:
//    server::Server* m_proxy_server;
//    std::shared_ptr<BS::thread_pool> m_thread_pool;
//    std::vector<std::unique_ptr<command::CommandBase>> m_command_list;
//};
//}