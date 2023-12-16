#pragma once
#include <string>

class Config {
public:
    struct HostConfig {
        std::uint16_t port;
    };

    struct ServerConfig {
        std::string host;
        std::string game_version;
        std::string platform_id;
        std::uint8_t protocol;
    };

    struct CommandConfig {
        std::string prefix;
    };

    struct MiscConfig {
        bool force_update_game_version;
        bool force_update_protocol;
        bool bypass_item_dat;
        bool spoof_login;
    };
public:
    void init();
    bool create(const std::string& file);
    bool load(const std::string& file);

public:
    HostConfig Host;
    ServerConfig Server;
    CommandConfig Command;
    MiscConfig Misc;
};
