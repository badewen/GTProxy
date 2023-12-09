#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

Config::Host s_host;
Config::Server s_server;
Config::Command s_command;
Config::Misc s_misc;

Config::Host Config::get_host() { return s_host; }
Config::Server Config::get_server() { return s_server; }
Config::Command Config::get_command() { return s_command; }
Config::Misc Config::get_misc() { return s_misc; }

void Config::Init()
{
    s_host = {};
    s_server = {};
    s_command = {};
    s_misc = {};

    // Initializes the configuration settings to default values.
    s_host.m_port = 16999;
    s_server.m_host = "www.growtopia1.com";
    s_server.m_game_version = "4.4";
    s_server.m_protocol = 191;
    s_server.m_platform_id = "4";
    s_command.m_prefix = "!";
    s_misc.m_force_update_game_version = false;
    s_misc.m_force_update_protocol = false;
    s_misc.m_bypass_item_dat = false;
    s_misc.m_spoof_login = false;
}

bool Config::Create(const std::string& file)
{
    nlohmann::json j{};
    j["host"]["port"] = s_host.m_port;
    j["server"]["host"] = s_server.m_host;
    j["server"]["gameVersion"] = s_server.m_game_version;
    j["server"]["protocol"] = s_server.m_protocol;
    j["server"]["platformID"] = s_server.m_platform_id;
    j["command"]["prefix"] = s_command.m_prefix;
    j["misc"]["forceUpdateGameVersion"] = s_misc.m_force_update_game_version;
    j["misc"]["forceUpdateGameVersionProtocol"] = s_misc.m_force_update_protocol;
    j["misc"]["bypassUpdateItemData"] = s_misc.m_bypass_item_dat;
    j["misc"]["spoofLogin"] = s_misc.m_spoof_login;

    std::ofstream ofs{ file };
    if (!ofs.is_open()) {
        spdlog::error("Failed to open config file.");
        return false;
    }

    ofs << std::setw(4) << j;
    return true;
}

bool Config::Load(const std::string& file)
{
    std::ifstream ifs{ file };
    if (!ifs.is_open()) {
        return Create(file);
    }

    nlohmann::json j{};
    ifs >> j;

    try {
        s_host.m_port = j["host"]["port"].get<std::uint16_t>();
        s_server.m_host = j["server"]["host"];
        s_server.m_game_version = j["server"]["gameVersion"];
        s_server.m_protocol = j["server"]["protocol"].get<std::uint8_t>();
        s_server.m_platform_id = j["server"]["platformID"];
        s_command.m_prefix = j["command"]["prefix"];
        s_misc.m_force_update_game_version = j["misc"]["forceUpdateGameVersion"];
        s_misc.m_force_update_protocol = j["misc"]["forceUpdateGameVersionProtocol"];
        s_misc.m_bypass_item_dat = j["misc"]["bypassUpdateItemData"];
        s_misc.m_spoof_login = j["misc"]["spoofLogin"];
    }
    catch (const nlohmann::json::exception& ex) {
        if (ex.id != 302) {
            spdlog::error("Configuration file \"{}\" failed to load.", file);
            spdlog::error("{}", ex.what());
            return false;
        }

        spdlog::warn("The configuration file \"{}\" is empty or contains a null value.", file);
        Create(file);
    }

    spdlog::info("Configuration file \"{}\" loaded successfully.", file);
    return true;
}
