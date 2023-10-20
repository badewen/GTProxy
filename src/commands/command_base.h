#pragma once

#include <vector>
#include <string>

#include "../client/client.h"

#include "../network/packet.h"

namespace command {
class CommandBase {
public:
    CommandBase( std::vector<std::string> command_aliases ) : m_aliases {command_aliases} {}

    virtual void Execute(client::Client* client, std::vector<std::string> args) = 0;

    bool FindAlias(std::string alias) { return std::find(m_aliases.begin(), m_aliases.end(), alias) != m_aliases.end(); }

private:
    std::vector<std::string> m_aliases;
};
}