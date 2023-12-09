#include "module_manager.h"

#include "modules/fast_drop_module.h"
#include "modules/fast_vend_module.h"
#include "modules/auto_fish_module.h"
#include "modules/world_handler_module.h"
#include "modules/white_skin_fix_module.h"

using namespace module;

ModuleManager::ModuleManager() {
    m_client = nullptr;
    m_modules.push_back(std::move(std::make_shared<modules::FastDropModule>(nullptr)));
    m_modules.push_back(std::move(std::make_shared<modules::FastVendModule>(nullptr)));
    m_modules.push_back(std::move(std::make_shared<modules::AutoFishModule>(nullptr)));
    m_modules.push_back(std::move(std::make_shared<modules::WorldHandlerModule>(nullptr)));
    m_modules.push_back(std::move(std::make_shared<modules::WhiteSkinFixModule>(nullptr)));
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

bool ModuleManager::enable_module(const std::string &module_name) {
    auto mod = get_module_by_name(module_name);

    if (!mod) {
        return false;
    }

    for (const auto& required_module : mod->RequiredModules) {
        if (!get_module_by_name(required_module)->Enabled) {
            return false;
        }
    }

    mod->enable();

    return true;
}

bool ModuleManager::disable_module(const std::string &module_name) {
    auto mod = get_module_by_name(module_name);

    if (!mod) {
        return false;
    }

    // recursively disable module that is depended.
    for (const auto& depended_module : m_modules) {
        for (const auto& required : depended_module->RequiredModules) {
            if (required == module_name) {
                disable_module(depended_module->Name);
                break;
            }
        }
    }

    mod->disable();

    return true;
}
