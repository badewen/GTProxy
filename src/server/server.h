#pragma once

#include "../http/http.h"
#include "../config.h"
#include "../enetwrapper/enet_server.h"
#include "../player/peer.h"
#include "../client/client.h"
#include "../utils/login_spoof_data.h"

#include "sigslot/signal.hpp"

namespace server {
class Server : public enet_wrapper::ENetServer {
public:
    explicit Server();
    ~Server();

    bool start();

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

public:
    bool is_gt_server_client_valid(player::Peer* gt_client) { return get_client_by_peer(gt_client)->is_valid(); }
    bool is_gt_client_valid(client::Client* server_client) { return get_peer_by_client(server_client) != nullptr
                                                                    && get_peer_by_client(server_client)->is_connected(); }

    void send_to_gt_client(client::Client* server_client, ENetPacket* packet) { is_gt_client_valid(server_client)
                                                                                    ? get_peer_by_client(server_client)
                                                                                        ->send_packet_packet(packet)
                                                                                    : 0; }
    enet_uint32 get_gt_client_connect_id(client::Client* server_client) { return get_peer_by_client(server_client)->get_connect_id(); }

    std::optional<utils::LoginSpoofData> get_login_spoof_data(const std::string& key);
    void set_login_spoof_data(const std::string& key, const utils::LoginSpoofData& data) { m_login_spoof_data_map
                                                                                            .insert_or_assign(key, data);
    }

    client::Client* get_client_by_peer(player::Peer* key);
    player::Peer* get_peer_by_client(client::Client* key);

private:
    player::Peer* get_gt_client_by_raw_peer(ENetPeer* key) { return m_gt_client_map.at(key); }

private:
    // peer is the growtopia client
    // Client is the "client" that interface directly with growtopia server.
    std::unordered_map<player::Peer*, client::Client*> m_client_map;

    // ENetPeer is the raw peer received from the enet
    // player::Peer is the layer abstraction of the raw peer.
    // can be used to access m_client_map
    // INTERNAL USE ONLY.
    std::unordered_map<ENetPeer*, player::Peer*> m_gt_client_map;

    // std::string can contain anything that uniquely identify a client even if the client reconnects.
    std::unordered_map<std::string, utils::LoginSpoofData> m_login_spoof_data_map;

};
}
