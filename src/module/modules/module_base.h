#pragma once

#include <string>
#include <utility>
#include <vector>

namespace client {
class Client;
}

namespace module {

class ModuleBase {
public:

    explicit ModuleBase(std::string module_name, std::vector<std::string> required_modules = {}) :
        Name {std::move( module_name )},
        RequiredModules {std::move(required_modules)}
    {}

    virtual void on_enable() {}
    virtual void on_disable() {}

    void update_curr_client(client::Client* client) {
        m_client = client;
    }

    void enable() {
        Enabled = true;
        on_enable();
    }

    void disable() {
        Enabled = false;
        on_disable();
    }

public:
    std::string Name;
    std::vector<std::string> RequiredModules;
    bool Enabled {};

protected:
    client::Client* m_client {};
};

}