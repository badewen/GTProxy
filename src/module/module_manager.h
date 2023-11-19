#pragma once

#include <vector>
#include <memory>

#include "modules/module_base.h"

namespace module {
class ModuleManager {
public:
    explicit ModuleManager();

    std::shared_ptr<module::ModuleBase> get_module_by_name(const std::string& module_name);
    void update_curr_client(client::Client* client);

private:
    client::Client* m_client;
    std::vector<std::shared_ptr<module::ModuleBase>> m_modules;

};
}