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
    [[nodiscard]] static const Host& get_host() { return s_host; }
    [[nodiscard]] static const Server& get_server() { return s_server; }
    [[nodiscard]] static const Command& get_command() { return s_command; }
    [[nodiscard]] static const Misc& get_misc() { return s_misc; }

private:
    static Host s_host;
    static Server s_server;
    static Command s_command;
    static Misc s_misc;
};
