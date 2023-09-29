#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "server.h"

namespace server {
Server::Server()
    : enet_wrapper::ENetServer{}
    , m_login_spoof_data_map {}
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

    server_client->start();
    gt_client->send_packet_packet(player::Peer::build_packet(player::NET_MESSAGE_SERVER_HELLO, std::vector<uint8_t>{1}));
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    player::Peer* gt_client = get_gt_client_by_raw_peer(peer);

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

    if (server_client->is_redirected()) return;

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

std::optional<utils::LoginSpoofData> Server::get_login_spoof_data(const std::string& key) {
    auto found = m_login_spoof_data_map.find(key);
    return found != m_login_spoof_data_map.end() ? std::optional<utils::LoginSpoofData>{found->second} : std::nullopt;
}

}
