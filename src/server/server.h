#pragma once

#include "../http/http.h"
#include "../config.h"
#include "../enetwrapper/enet_server.h"
#include "../player/peer.h"
#include "../client/client.h"

namespace server {
class Server : public enet_wrapper::ENetServer {
public:
    explicit Server();
    ~Server();

    bool start();

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    bool process_packet(ENetPeer* peer, ENetPacket* packet);
    bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet) const;

public:
    player::Peer* get_gt_client_peer() { return m_gt_client; }
    bool is_gt_server_client_valid() { return m_gt_server_client->is_valid(); }
    bool is_gt_client_valid() { return m_gt_client && m_gt_client->is_connected(); }
    void send_to_gt_client(ENetPacket* packet) { m_gt_client && m_gt_client->is_connected() ? m_gt_client->send_packet_packet(packet) : 0; }
    enet_uint32 get_gt_client_connect_id() const { return m_gt_client->get_connect_id(); }

private:
    client::Client* m_gt_server_client;
    player::Peer* m_gt_client{};

    // just a convenient way
    struct {
        player::Peer* m_gt_server_client;
    } m_peer;

};
}
