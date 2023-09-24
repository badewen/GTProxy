#pragma once

#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../player/peer.h"
#include "../eventmanager/event_manager.h"

namespace server {
class Server;
}

namespace client {
class Client : public enet_wrapper::ENetClient {
    friend server::Server;
public:
    explicit Client(server::Server* server);
    ~Client();

    void start();

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    bool process_packet(ENetPeer* peer, ENetPacket* packet);
    bool process_raw_packet(ENetPeer* peer, ENetPacket* packet);
    bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);
    bool process_variant_list(ENetPeer* peer, VariantList* packet);

public:
    void send_to_gt_server(ENetPacket* packet) { m_gt_server  && m_gt_server->is_connected() ? m_gt_server->send_packet_packet(packet) : 0; };
    bool is_valid() { return m_gt_server && m_gt_server->is_connected(); }

private:
    server::Server* m_proxy_server;
    player::Peer* m_gt_server{};

    struct {
        enet_uint8 m_using_new_packet;
        std::string m_host;
        enet_uint16 m_port;
    } m_redirect_server;
};
}
