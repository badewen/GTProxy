#include <filesystem>
#include <thread>
#include <spdlog/spdlog.h>
#include <magic_enum.hpp>

#include "http.h"
#include "ssl.h"
#include "../utils/text_parse.h"
#include "../utils/ip.h"
#include "../ipresolver/resolver.h"

namespace server {
std::unique_ptr<httplib::Server> Http::s_server {};
httplib::Headers Http::s_last_headers {};
httplib::Params Http::s_last_params {};
std::string Http::s_last_content {};
std::unordered_map<std::string, utils::TextParse> Http::ServerDataCache {};

void Http::Init()
{
    std::string cache_dir{ "./cache" };
    std::filesystem::create_directory(cache_dir);

    const auto cert_path = cache_dir + "/cert.pem";
    const auto key_path = cache_dir + "/key.pem";

    if (!std::filesystem::exists(cert_path)) {
        std::ofstream cert_file{ cert_path };
        cert_file << ssl::cert;
        cert_file.close();
    }

    if (!std::filesystem::exists(key_path)) {
        std::ofstream key_file{ key_path };
        key_file << ssl::key;
        key_file.close();
    }

    s_server = std::make_unique<httplib::SSLServer>("./cache/cert.pem", "./cache/key.pem");
}

bool Http::bind_to_port(const std::string& host, int port)
{
    if (s_server->bind_to_port(host, port)) {
        // So we don't need to store port in a member variable.
        spdlog::info("HTTP(s) server listening on port {}.", port);
        return true;
    }

    spdlog::error("Failed to bind to port {}.", port);
    return false;
}

void Http::listen_after_bind()
{
    std::thread{ &Http::listen_internal}.detach();
}

bool Http::listen(const std::string& host, int port)
{
    if (!bind_to_port(host, port)) {
        return false;
    }

    listen_after_bind();
    return true;
}

void Http::stop()
{
    s_server->stop();
}

void Http::listen_internal()
{
    s_server->set_logger([](const httplib::Request& req, const httplib::Response& res) {
        spdlog::info("{} {} {}", req.method, req.path, res.status);
    });

    s_server->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_content(
            fmt::format(
                "Hello, world!\r\n{} ({})",
                httplib::detail::status_message(res.status),
                res.status
            ),
            "text/plain"
        );
    });

    s_server->set_exception_handler([](
        const httplib::Request& req,
        httplib::Response& res,
        const std::exception_ptr& ep
    ) {
        res.status = 500;

        try {
            std::rethrow_exception(ep);
        }
        catch (std::exception &e) {
            res.set_content(fmt::format("Hello, world!\r\n{}", e.what()), "text/plain");
        }
        catch (...) {
            res.set_content("Hello, world!\r\nUnknown Exception", "text/plain");
        }
    });

    s_server->Post("/growtopia/server_data.php", [&](
        const httplib::Request& req,
        httplib::Response& res
    ) {
        if (!req.headers.empty()) {
            spdlog::info("Headers:");
            for (auto& header : req.headers) {
                spdlog::info("\t{}: {}", header.first, header.second);
            }
        }

        if (!req.params.empty()) {
            spdlog::info("Params:");
            spdlog::info("\t{}", httplib::detail::params_to_query_str(req.params));
        }

        if (!req.body.empty()) {
            spdlog::info("Contents:");
            spdlog::info("\t{}", req.body);
        }

        s_last_headers = req.headers;
        s_last_params = req.params;
        s_last_content = req.body;

        utils::TextParse text_parse{ get_server_data() };
        ServerDataCache.insert_or_assign(text_parse.get("meta", 1), text_parse);

        spdlog::info("Server Returned:");
        spdlog::info("\t{}", text_parse.get_all_raw());

        text_parse.set("server", "127.0.0.1");
        text_parse.set("port", Config::get_host().m_port);

        res.set_content(text_parse.get_all_raw(), "text/html");
        return true;
    });

    s_server->listen_after_bind();
}

std::string Http::get_server_data()
{
    spdlog::debug("Requesting server data from: https://{}", Config::get_server().m_host);

    auto validate_server_response{
        [](const httplib::Result& response)
        {   
            httplib::Error error_response{ response.error() };

            if (!response) {
                spdlog::error(
                    "Response is null with error: httplib::Error::{}",
                    magic_enum::enum_name(error_response));

                return false;
            }

            int status_code{ response->status };

            if (error_response == httplib::Error::Success && status_code == 200) {
                return true;
            }

            spdlog::error(
                "Failed to get server data. {}.",
                error_response == httplib::Error::Success
                    ? fmt::format("HTTP status code: {}", status_code)
                    : fmt::format("HTTP error: {}", httplib::to_string(error_response)));
            return false;
        }
    };

    std::string resolved_ip = Config::get_server().m_host;

    if (utils::IsIpOrHostname(Config::get_server().m_host) == utils::HostType::Hostname) {
        auto res = Resolver::ResolveHostname(Config::get_server().m_host);

        if (res.Statuz != Resolver::NoError) {
            spdlog::error(
                "Error occurred while resolving {} ip address. Dns server returned {}",
                Config::get_server().m_host,
                magic_enum::enum_name(res.Statuz));
            return {};
        }

        resolved_ip = res.Ip;

        spdlog::info(
            "{} ip address is {}",
            Config::get_server().m_host,
            resolved_ip);
    }

    httplib::Client cli{ fmt::format("https://{}", resolved_ip) };
    cli.enable_server_certificate_verification(false);

    httplib::Headers header {
        {"User-Agent", get_header_value(s_last_headers, "User-Agent")},
        {"Host", Config::get_server().m_host}
    };

    httplib::Result response{ cli.Post(
        "/growtopia/server_data.php",
        header,
        s_last_content,
        "application/x-www-form-urlencoded"
    ) };
    if (validate_server_response(response)) {
        if (!response->body.empty()) {
            // utils::TextParse text_parse{ response->body };
            // spdlog::debug("Server data: \r\n{}", fmt::join(text_parse.get_all_array(), "\r\n"));
            return response->body;
        }
    }

    // If Post request fails, try Get request.
    response = cli.Get("/growtopia/server_data.php");
    if (validate_server_response(response)) {
        if (!response->body.empty()) {
            // utils::TextParse text_parse{ response->body };
            // spdlog::debug("Server data: \r\n{}", fmt::join(text_parse.get_all_array(), "\r\n"));
            return response->body;
        }
    }

    spdlog::warn("Failed to retrieve server data from {}", Config::get_server().m_host);
    return {};
}
}
