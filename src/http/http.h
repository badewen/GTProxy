#pragma once
#include <string>
#include <httplib/httplib.h>

#include "../config.h"

namespace server {
class Http {
public:
    static void Init();
    static bool bind_to_port(const std::string& host, int port);
    static void listen_after_bind();
    static bool listen(const std::string& host, int port);
    static void stop();

private:
    static void listen_internal();

public:
    static std::string get_server_data();

private:
    static std::unique_ptr<httplib::Server> s_server;
    static httplib::Headers s_last_headers;
    static httplib::Params s_last_params;
};
}
