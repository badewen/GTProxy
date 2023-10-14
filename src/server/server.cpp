#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "server.h"

namespace server {
Server::Server()
    : enet_wrapper::ENetServer{}
{
    m_client_map = {};
}

Server::~Server()
{
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

    auto gt_client = new player::Peer{ peer };
    auto server_client = new client::Client{ this };

    m_client_map.emplace(gt_client, server_client);
    m_gt_client_map.emplace(peer, gt_client);

    gt_client->send_packet(player::NET_MESSAGE_SERVER_HELLO, { 0 });
    spdlog::debug("SENT SERVER HELLO PACKET");
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::get_message_type(packet) };
    player::Peer* gt_client = get_gt_client_by_raw_peer(peer);

    if (message_type == player::NET_MESSAGE_GENERIC_TEXT) {
        std::string message_data{ player::get_text(packet) };

        if (message_data.find("requestedName") != std::string::npos) {
            auto login_text_parse = utils::TextParse(message_data);

            // generate a key for each unique connected client
            auto generate_spoof_map_key = [](utils::TextParse parse) -> std::string {
                return utils::hash::sha256(
                        parse.get("klv", 1) +
                        parse.get("requestedName", 1) +
                        parse.get("tankIDName", 1) +
                        parse.get("tankIDPass", 1)
                );
            };

            // find the context
            auto it = m_client_context_map.find(generate_spoof_map_key(login_text_parse));
            std::shared_ptr<client::ClientContext> ctx {};

            // if context is not found, create new one
            if (it == m_client_context_map.end()) {
                spdlog::debug("Creating new Context..");
                ctx = std::make_shared<client::ClientContext>();

                // this cant fail; else crash.
                utils::TextParse http_data_text_parse = server::Http::ServerDataCache.at(login_text_parse.get("meta", 1));

                ctx->RedirectIp     = http_data_text_parse.get("server", 1);
                ctx->RedirectPort   = http_data_text_parse.get<enet_uint16>("port", 1);
                ctx->LoginSpoofData = utils::LoginSpoofData::Generate();

                m_client_context_map.insert_or_assign(generate_spoof_map_key(login_text_parse), ctx);
            }
            else { ctx = it->second; }

            ctx->LoginData = login_text_parse.get_all_raw();

            // start the server client.
            spdlog::debug("Starting a new server client..");
            get_client_by_peer(m_gt_client_map.at(peer))->start(ctx);
            return;
        }
    }

    if (!is_gt_server_client_valid(gt_client)) {
        return;
    }

    get_client_by_peer(gt_client)->queue_outgoing_packet(packet);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server.");

    player::Peer* server_client_peer = get_client_by_peer( get_gt_client_by_raw_peer(peer) )
                                            ->to_peer();

    if (server_client_peer && server_client_peer->is_connected()) {
        server_client_peer->disconnect();
    }

    player::Peer* gt_client = get_gt_client_by_raw_peer(peer);
    client::Client* server_client = get_client_by_peer(gt_client);

    m_gt_client_map.erase(peer);
    m_client_map.erase(gt_client);

    delete gt_client;
    delete server_client;
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
