#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

void Config::init()
{
    this->Host = {};
    this->Server = {};
    this->Command = {};
    this->Misc = {};

    // Initializes the configuration settings to default values.
    this->Host.port = 16999;
    this->Server.host = "www.growtopia1.com";
    this->Server.game_version = "4.4";
    this->Server.protocol = 191;
    this->Server.platform_id = "4";
    this->Command.prefix = "!";
    this->Misc.force_update_game_version = false;
    this->Misc.force_update_protocol = false;
    this->Misc.bypass_item_dat = false;
    this->Misc.spoof_login = false;
}

bool Config::create(const std::string& file)
{
    nlohmann::json j{};
    j["host"]["port"] = this->Host.port;
    j["server"]["host"] = this->Server.host;
    j["server"]["gameVersion"] = this->Server.game_version;
    j["server"]["protocol"] = this->Server.protocol;
    j["server"]["platformID"] = this->Server.platform_id;
    j["command"]["prefix"] = this->Command.prefix;
    j["misc"]["forceUpdateGameVersion"] = this->Misc.force_update_game_version;
    j["misc"]["forceUpdateGameVersionProtocol"] = this->Misc.force_update_protocol;
    j["misc"]["bypassUpdateItemData"] = this->Misc.bypass_item_dat;
    j["misc"]["spoofLogin"] = this->Misc.spoof_login;

    std::ofstream ofs{ file };
    if (!ofs.is_open()) {
        spdlog::error("Failed to open config file.");
        return false;
    }

    ofs << std::setw(4) << j;
    return true;
}

bool Config::load(const std::string& file)
{
    std::ifstream ifs{ file };
    if (!ifs.is_open()) {
        return create(file);
    }

    nlohmann::json j{};
    ifs >> j;

    try {
        this->Host.port = j["host"]["port"].get<std::uint16_t>();
        this->Server.host = j["server"]["host"];
        this->Server.game_version = j["server"]["gameVersion"];
        this->Server.protocol = j["server"]["protocol"].get<std::uint8_t>();
        this->Server.platform_id = j["server"]["platformID"];
        this->Command.prefix = j["command"]["prefix"];
        this->Misc.force_update_game_version = j["misc"]["forceUpdateGameVersion"];
        this->Misc.force_update_protocol = j["misc"]["forceUpdateGameVersionProtocol"];
        this->Misc.bypass_item_dat = j["misc"]["bypassUpdateItemData"];
        this->Misc.spoof_login = j["misc"]["spoofLogin"];
    }
    catch (const nlohmann::json::exception& ex) {
        if (ex.id != 302) {
            spdlog::error("Configuration file \"{}\" failed to load.", file);
            spdlog::error("{}", ex.what());
            return false;
        }

        spdlog::warn("The configuration file \"{}\" is empty or contains a null value.", file);
        create(file);
    }

    spdlog::info("Configuration file \"{}\" loaded successfully.", file);
    return true;
}
