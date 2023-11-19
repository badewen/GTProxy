#pragma once

#include <string>

namespace client {
class Client;
}

namespace module {

class ModuleBase {
public:

    ModuleBase(std::string module_name) :
        Name{ module_name }
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
    bool Enabled;

protected:
    client::Client* m_client;
};

}