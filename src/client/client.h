#pragma once

#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../player/peer.h"

#include "../utils/hash.h"
#include "rwqueue/readerwriterqueue.h"
#include "../utils/randutils.hpp"
#include "../utils/random.h"

namespace server {
class Server;
}

namespace client {
class Client : public enet_wrapper::ENetClient {
public:
    explicit Client(server::Server* server);
    ~Client();

    void start();

    void on_connect(ENetPeer* peer) override;
    void on_service_loop() override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    bool process_incoming_packet(ENetPacket* packet);
    bool process_incoming_raw_packet(player::GameUpdatePacket* game_update_packet);
    bool process_incoming_variant_list(VariantList* packet, uint32_t net_id);

    bool process_outgoing_packet(ENetPacket* packet);
    bool process_outgoing_raw_packet(player::GameUpdatePacket* game_update_packet);

    void send_to_server(ENetPacket* packet);
    void send_to_gt_client(ENetPacket* packet);

public:
    bool is_valid() { return m_peer_wrapper && m_peer_wrapper->is_connected(); }
    bool is_redirected() const { return m_redirected; }
    void queue_outgoing_packet(ENetPacket* packet) { m_outgoing_packet_queue.enqueue(packet); }

private:
    server::Server* m_proxy_server;
    // so we dont need to fetch it every single time.
    player::Peer* m_gt_client;

    moodycamel::ReaderWriterQueue<ENetPacket*> m_outgoing_packet_queue;

    std::string m_login_data;
    bool m_send_login_data;
    bool m_redirected;

    struct {
        enet_uint8 m_using_new_packet;
        std::string m_host;
        enet_uint16 m_port;
        enet_uint32 m_token, m_user, m_door_id;
        std::string m_uuid_token;
    } m_redirect_server;

};
}
