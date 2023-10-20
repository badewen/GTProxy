#pragma once

#include <sigslot/signal.hpp>
#include <concurrentqueue/concurrentqueue.h>

#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../peer/peer.h"

#include "../utils/hash.h"
#include "../utils/randutils.hpp"
#include "../utils/random.h"
#include "../utils/text_parse.h"
#include "../utils/login_spoof_data.h"
#include "../world/world.h"
#include "../player/player.h"
#include "../utils/timer.h"

namespace server {
class Server;
}

namespace client {

struct ClientContext {
    std::string RedirectIp;
    enet_uint16 RedirectPort;
    bool IsLoginDataSent;
    utils::LoginSpoofData LoginSpoofData;
    std::string LoginData;
    player::Peer* GtClientPeer;
};

struct PacketInfo {
    ENetPacket* Packet;
    bool IsOutgoing;
    bool ShouldProcess;
    utils::Timer Delay;
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
    bool process_incoming_raw_packet(packet::GameUpdatePacket* game_update_packet);
    bool process_incoming_variant_list(VariantList* packet, int32_t net_id);

    bool process_outgoing_packet(ENetPacket* packet);
    bool process_outgoing_raw_packet(packet::GameUpdatePacket* game_update_packet);

    void send_to_server(ENetPacket* packet);
    void send_to_server_delayed(ENetPacket* packet, float delay_ms) { queue_packet_delayed(packet, true, delay_ms, false); }
    void send_to_gt_client(ENetPacket* packet);
    void send_to_gt_client_delayed(ENetPacket* packet, float delay_ms) { queue_packet_delayed(packet, false, delay_ms, false); };


public:
    bool is_valid() { return m_peer_wrapper && m_peer_wrapper->is_connected(); }
    bool is_ctx_empty() { return !m_ctx; }
    void queue_packet(ENetPacket* packet, bool is_outgoing, bool should_process = true) { queue_packet_delayed(packet, is_outgoing, 0, should_process); }
    void queue_packet_delayed(ENetPacket* packet, bool is_outgoing, float delay_ms, bool should_process = true) {
        m_packet_queue.enqueue({packet, is_outgoing, should_process, delay_ms}); }

private:
    server::Server* m_proxy_server;

    std::shared_ptr<ClientContext> m_ctx;

    // bool = is outgoing packet
    // why? to equalize the outgoing and incoming packet's priority ( I LOVE DEMOCRACY AND EQUAL RIGHT RAHHHHH -̶n̶o̶t̶ ̶b̶e̶i̶n̶g̶ ̶h̶e̶l̶d̶ ̶g̶u̶n̶p̶o̶i̶n̶t̶ ̶a̶t̶ )
    moodycamel::ConcurrentQueue<PacketInfo> m_packet_queue;

    World m_curr_world;
    Player m_curr_player;

    sigslot::signal<ENetPacket* /* packet */, bool* /* should_forward_packet */> m_on_incoming_packet;
    sigslot::signal<ENetPacket* /* packet */, bool* /* should_forward_packet */> m_on_outgoing_packet;

    sigslot::signal<packet::GameUpdatePacket* /* tank_packet */, bool* /* should_forward_packet */> m_on_incoming_tank_packet;
    sigslot::signal<packet::GameUpdatePacket* /* tank_packet */, bool* /* should_forward_packet */> m_on_outgoing_tank_packet;

    sigslot::signal<VariantList* /* varlist */, int32_t /* net_id */, bool*> m_on_incoming_varlist;

};
}
