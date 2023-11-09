#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "server.h"

namespace server {
Server::Server()
    : enet_wrapper::ENetServer{}
    , m_client_pool{}
{
    m_client_map = {};
    m_thread_pool = std::make_shared<BS::thread_pool>();
    spdlog::debug("NEW SERVER CLASS IS CREATED");
}

Server::~Server()
{
    for (auto& kv : m_gt_client_map) {
        player::Peer* server_client_peer = get_client_by_peer(get_gt_client_by_raw_peer(kv.first) )
                ->to_peer();

        if (server_client_peer && server_client_peer->is_connected()) {
            server_client_peer->disconnect();
        }

        player::Peer* gt_client = get_gt_client_by_raw_peer(kv.first);
        gt_client->disconnect_now();

        delete gt_client;
    }

    spdlog::debug("SERVER CLASS IS DESTROYED");
}

bool Server::start()
{
    if (!create_host(Config::get_host().m_port, 1, 1)) {
        spdlog::error("Failed to create ENet server host.");
        return false;
    }

    start_service();
    spdlog::info("ENet server listening on port {}.", Config::get_host().m_port);
    return true;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info("New client connected to proxy server.");

    auto gt_client = new player::Peer{peer };
    client::Client* server_client {};

    for (auto& client : m_client_pool) {
        if (client->is_ctx_empty()) {
            server_client = client.get();
        }
    }

    if (!server_client) {
        auto temp = std::make_shared<client::Client>(this, m_thread_pool);
        server_client = temp.get();
        m_client_pool.push_back(temp);
    }

    m_client_map.emplace(gt_client, server_client);
    m_gt_client_map.emplace(peer, gt_client);

    gt_client->send_packet(packet::NET_MESSAGE_SERVER_HELLO, { 0 });
    spdlog::debug("SENT SERVER HELLO PACKET TO GT CLIENT");
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    packet::eNetMessageType message_type{packet::get_message_type(packet) };
    player::Peer* gt_client = get_gt_client_by_raw_peer(peer);

    if (message_type == packet::NET_MESSAGE_GENERIC_TEXT) {
        std::string message_data{packet::get_text(packet) };

        if (message_data.find("requestedName") != std::string::npos) {
            auto login_text_parse = utils::TextParse(message_data);

            // generate a key for each unique connected client
            auto generate_context_key = [](utils::TextParse parse) -> std::string {
                return utils::hash::sha256(
                        parse.get("klv", 1) +
                        parse.get("requestedName", 1) +
                        parse.get("tankIDName", 1)
                );
            };

            // find the context
            auto found_ctx = m_client_context_map.find(generate_context_key(login_text_parse));
            std::shared_ptr<client::ClientContext> ctx {};

            // if context is not found, create new one
            if (found_ctx == m_client_context_map.end()) {
                spdlog::debug("Creating new Context..");
                ctx = std::make_shared<client::ClientContext>();

                ctx->LoginSpoofData = utils::LoginSpoofData::Generate();

                m_client_context_map.insert_or_assign(generate_context_key(login_text_parse), ctx);
            }
            else {
                ctx = found_ctx->second;
                if (ctx->IsConnected) {
                    spdlog::warn("SAME LOGIN DATA DETECTED");
                    gt_client->send_packet_packet(player::Peer::build_variant_packet(
                            {"OnConsoleMessage",
                             "[`2GTPROXY`o] `4Oops login can't proceed`0. It seems like the LoginData is the same, but the client hasn't diconnected yet."},
                            -1, ENET_PACKET_FLAG_RELIABLE), true);

                    gt_client->send_packet_packet(player::Peer::build_variant_packet(
                            {"OnConsoleMessage",
                             "[`2GTPROXY`o] if there are another connected guest account on the same device, try disconnecting it or login using a GrowID."},
                            -1, ENET_PACKET_FLAG_RELIABLE), true);
                    gt_client->disconnect_now();
                    return;
                }
            }

            auto found_http_data = server::Http::ServerDataCache.find(login_text_parse.get("meta", 1));

            if (found_http_data != server::Http::ServerDataCache.end()) {
                ctx->RedirectIp     = found_http_data->second.get("server", 1);
                ctx->RedirectPort   = found_http_data->second.get<enet_uint16>("port", 1);

                server::Http::ServerDataCache.erase(login_text_parse.get("meta", 1));
            }

            ctx->LoginData          = login_text_parse.get_all_raw();
            ctx->IsLoginDataSent    = false;
            ctx->IsConnected        = true;
            ctx->GtClientPeer       = gt_client;

            // start the server client.
            spdlog::debug("Starting a new server client..");
            m_thread_pool->push_task(&client::Client::start, get_client_by_peer(gt_client), ctx);
            return;
        }
    }

    if (!is_gt_server_client_valid(gt_client)) {
        return;
    }

    get_client_by_peer(gt_client)->queue_packet(packet, true);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server.");

    player::Peer* server_client_peer = get_client_by_peer(get_gt_client_by_raw_peer(peer) )
                                            ->to_peer();

    if (server_client_peer && server_client_peer->is_connected()) {
        server_client_peer->disconnect();
    }

    player::Peer* gt_client = get_gt_client_by_raw_peer(peer);

    m_gt_client_map.erase(peer);
    m_client_map.erase(gt_client);

    delete gt_client;
}

player::Peer* Server::get_peer_by_client(client::Client *key) {
    player::Peer* ret = nullptr;

    std::for_each(m_client_map.begin(), m_client_map.end(), [&](const std::pair<player::Peer*, client::Client*>& i) {
        if (i.second == key) {
            ret = i.first;
            return false;
        }
        return true;
    });

    return ret;
}

client::Client* Server::get_client_by_peer(player::Peer *key) {
    auto pos = m_client_map.find(key);
    if (pos == m_client_map.end()) {
        return nullptr;
    }
    return pos->second;
}

}
