#pragma once
#include <string>

class Config {
public:
    struct Host {
        std::uint16_t m_port;
    };

    struct Server {
        std::string m_host;
        std::string m_game_version;
        std::string m_platform_id;
        std::uint8_t m_protocol;
    };

    struct Command {
        std::string m_prefix;
    };

    struct Misc {
        bool m_force_update_game_version;
        bool m_force_update_protocol;
    };
public:
    static void Init();
    static bool Create(const std::string& file);
    static bool Load(const std::string& file);

public:
    static Host get_host();
    static Server get_server();
    static Command get_command();
    static Misc get_misc();
};
