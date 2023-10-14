#pragma once

#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../player/peer.h"

#include "../utils/hash.h"
#include "rwqueue/readerwriterqueue.h"
#include "../utils/randutils.hpp"
#include "../utils/random.h"
#include "../utils/text_parse.h"
#include "../utils/login_spoof_data.h"

namespace server {
class Server;
}

namespace client {

struct ClientContext {
    std::string RedirectIp;
    enet_uint16 RedirectPort;
    utils::LoginSpoofData LoginSpoofData;
    std::string LoginData;
    player::Peer* GtClientPeer;
};

class Client : public enet_wrapper::ENetClient {
public:
    explicit Client(server::Server* server);
    ~Client();

    // server's responsibility to fetch the right Context
    // or create new one if necessary.
    // ctx cant be null
    void start(std::shared_ptr<ClientContext> ctx);

    void on_connect(ENetPeer* peer) override;
    void on_service_loop() override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    bool process_incoming_packet(ENetPacket* packet);
    bool process_incoming_raw_packet(player::GameUpdatePacket* game_update_packet);
    bool process_incoming_variant_list(VariantList* packet, int32_t net_id);

    bool process_outgoing_packet(ENetPacket* packet);
    bool process_outgoing_raw_packet(player::GameUpdatePacket* game_update_packet);

    void send_to_server(ENetPacket* packet);
    void send_to_gt_client(ENetPacket* packet);

public:
    bool is_valid() { return m_peer_wrapper && m_peer_wrapper->is_connected(); }
    void queue_outgoing_packet(ENetPacket* packet) { m_outgoing_packet_queue.enqueue(packet); }

private:
    server::Server* m_proxy_server;

    std::shared_ptr<ClientContext> m_ctx;

    moodycamel::ReaderWriterQueue<ENetPacket*> m_outgoing_packet_queue;

};
}
