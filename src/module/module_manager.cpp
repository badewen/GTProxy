#include "module_manager.h"

#include "modules/fast_drop_module.h"

using namespace module;

ModuleManager::ModuleManager() {
    m_client = nullptr;
    m_modules.push_back(std::move(std::make_shared<modules::FastDropModule>(nullptr)));
}

std::shared_ptr<module::ModuleBase> ModuleManager::get_module_by_name(const std::string &module_name) {
    for (auto& mod : m_modules) {
        if (mod->Name == module_name)
            return mod;
    }
    return nullptr;
}

void ModuleManager::update_curr_client(client::Client *client) {
    for (auto& mod : m_modules) {
        mod->update_curr_client(client);
    }
    m_client = client;
}
