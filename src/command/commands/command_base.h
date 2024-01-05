//#pragma once
//
//#include <utility>
//#include <vector>
//#include <string>
//#include <algorithm>
//
//#include "../../packet/packet.h"
//#include "../../config.h"
//
//namespace server {
//class Server;
//}
//
//namespace command {
//
//class CommandBase {
//public:
//    explicit CommandBase( std::vector<std::string>&& command_aliases,
//                          std::vector<std::string>&& command_args = {},
//                          std::string&& command_desc = "No Description",
//                          uint16_t min_args = 0,
//                          bool threaded = false)
//        : m_aliases { std::move(command_aliases) }
//        , m_desc { command_desc }
//        , IsThreaded { threaded }
//        , m_args { command_args }
//        , m_minimum_args_count { min_args }
//    {
//        assert(m_aliases.size() >= 1 && "command name is empty!");
//    }
//    CommandBase() = default;
//    virtual ~CommandBase() = default;
//
//    virtual void execute(server::Server* proxy_server, std::vector<std::string> args) = 0;
//
//    bool find_alias(const std::string& alias) { return std::find(m_aliases.begin(), m_aliases.end(), alias) != m_aliases.end(); }
//
//    void set_current_proxy_server(server::Server* proxy_server) {
//        m_proxy_server = proxy_server;
//    }
//
//    // the first value of m_aliases is used to refer to command name
//    std::string get_usage_hint() {
//        std::string temp = m_aliases[0];
//        std::for_each(m_args.begin(), m_args.end(), [&](const std::string& arg) {
//            temp.append(fmt::format(" <{}>", arg));
//        });
//        return temp;
//    }
//
//    std::string get_desc() {
//        return m_desc;
//    }
//
//    size_t get_args_count() { return m_args.size(); }
//    uint16_t get_minimum_args_count() const { return m_minimum_args_count; }
//
//public:
//    bool IsThreaded{};
//private:
//    std::vector<std::string> m_aliases;
//    std::string m_desc;
//
//    server::Server* m_proxy_server;
//
//    // if the args contains three trailing dots ("...") or m_args is empty
//    // ex : { "a", "b..." } or m_args.empty() is true
//    // then the "m_vararg" flag is set to true.
//    std::vector<std::string> m_args;
//
//    uint16_t m_minimum_args_count{};
//};
//}