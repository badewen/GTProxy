#pragma once

#include <utility>
#include <vector>
#include <string>
#include <algorithm>

#include "../../network/packet.h"
#include "../../config.h"

namespace client {
    class Client;
}

namespace command {

class CommandBase {
public:
    explicit CommandBase( std::vector<std::string>&& command_aliases,
                          std::vector<std::string>&& command_args = {},
                          std::string&& command_desc = "No Description",
                          bool threaded = false,
                          bool bypass_arg_check = false)
        : m_aliases { std::move(command_aliases) }
        , m_desc { command_desc }
        , IsThreaded { threaded }
        , m_args { command_args }
        , m_bypass_arg_check { bypass_arg_check }
    {
        assert(m_aliases.size() >= 1 && "command name is empty!");

        m_varArg = m_args.empty() || m_args.at(m_args.size() - 1).ends_with("...");
    }
    CommandBase() = default;
    virtual ~CommandBase() = default;

    virtual void execute(client::Client* client, std::vector<std::string> args) = 0;

    bool find_alias(const std::string& alias) { return std::find(m_aliases.begin(), m_aliases.end(), alias) != m_aliases.end(); }

    // the first value of m_aliases is used to refer to command name
    std::string get_usage_hint() {
        std::string temp = Config::get_command().m_prefix + m_aliases[0];
        std::for_each(m_args.begin(), m_args.end(), [&](const std::string& arg) {
            temp.append(fmt::format(" <{}>", arg));
        });
        return temp;
    }

    std::string get_desc() {
        return m_desc;
    }

    uint32_t get_args_count() { return m_args.size(); }
    bool is_var_arg() { return m_varArg; }
    bool should_bypass_arg_check() { return m_bypass_arg_check; }

public:
    bool IsThreaded;
private:
    std::vector<std::string> m_aliases;
    std::string m_desc;

    // if the args contains three trailing dots ("...") or m_args is empty
    // ex : { "a", "b..." } or m_args.empty() is true
    // then the "m_vararg" flag is set to true.
    std::vector<std::string> m_args;

    // if this flag is set, the number of arguments is not defined or a variable argument as in c++.
    bool m_varArg;

    bool m_bypass_arg_check;
};
}