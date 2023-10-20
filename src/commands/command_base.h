#pragma once

#include <utility>
#include <vector>
#include <string>

#include "../network/packet.h"

namespace client {
    class Client;
}

namespace command {
class CommandBase {
public:
    explicit CommandBase( std::vector<std::string> command_aliases ) : m_aliases { std::move(command_aliases) } {}
    virtual ~CommandBase() = default;

    virtual void Execute(client::Client* client, std::vector<std::string> args) = 0;

    bool FindAlias(std::string alias) { return std::find(m_aliases.begin(), m_aliases.end(), alias) != m_aliases.end(); }

private:
    std::vector<std::string> m_aliases;
};
}