#pragma once

#include <vector>
#include <memory>

#include "modules/module_base.h"

namespace module {
class ModuleManager {
public:
    explicit ModuleManager();

    std::shared_ptr<module::ModuleBase> get_module_by_name(const std::string& module_name);
    // safe way to enable/disable module.
    bool enable_module(const std::string& module_name);
    // disable modules that has dependency on the module that is being disabled.
    bool disable_module(const std::string& module_name);

    void update_curr_client(client::Client* client);

    void inline enable_all_module() {
        for (auto& mod : m_modules) {
            mod->enable();
        }
    }

    void inline disable_all_module() {
        for (auto& mod : m_modules) {
            mod->disable();
        }
    }

private:
    client::Client* m_client;
    std::vector<std::shared_ptr<module::ModuleBase>> m_modules;

};
}