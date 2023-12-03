#pragma once

#include <sigslot/signal.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <threadpool/include/BS_thread_pool.hpp>

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
#include "../utils/event_manager.h"
#include "../command/command_manager.h"
#include "../module/module_manager.h"

namespace server {
class Server;
}

namespace client {

struct ClientContext {
    std::string RedirectIp;
    enet_uint16 RedirectPort;
    bool IsLoginDataSent;
    bool IsConnected;
    bool UseModifiedENet;
    utils::LoginSpoofData LoginSpoofData;
    std::string LoginData;
    player::Peer* GtClientPeer;
    module::ModuleManager ModuleMgr;

    utils::EventManager<ENetPacket* /* packet */, bool* /* should_forward_packet */> OnIncomingPacket;
    utils::EventManager<ENetPacket* /* packet */, bool* /* should_forward_packet */> OnOutgoingPacket;

    utils::EventManager<packet::GameUpdatePacket* /* tank_packet */, bool* /* should_forward_packet */> OnIncomingTankPacket;
    utils::EventManager<packet::GameUpdatePacket* /* tank_packet */, bool* /* should_forward_packet */> OnOutgoingTankPacket;

    utils::EventManager<VariantList* /* varlist */, int32_t /* net_id */, bool*> OnIncomingVarlist;
};

struct PacketInfo {
    ENetPacket* Packet;
    bool IsOutgoing;
    bool ShouldProcess;
    utils::Timer Delay;
};

class Client : public enet_wrapper::ENetClient {
public:
    // thread_pool here is usually provided by server.cpp, because creating new thread pool is expensive.
    explicit Client(server::Server* server, std::shared_ptr<BS::thread_pool> thread_pool);
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
    void send_to_gt_client(ENetPacket* packet, bool destroy_packet = false);
    void send_to_gt_client_delayed(ENetPacket* packet, float delay_ms) { queue_packet_delayed(packet, false, delay_ms, false); };

    void log_to_client(const std::string& message);

public:
    bool is_valid() { return m_peer_wrapper && m_peer_wrapper->is_connected(); }
    bool is_ctx_empty() { return !m_ctx; }

    std::shared_ptr<ClientContext> get_ctx() { return m_ctx; }
    Player& get_local_player() { return m_curr_player; }
    World& get_current_world() { return m_curr_world; }

    void queue_packet(ENetPacket* packet, bool is_outgoing, bool should_process = true) { queue_packet_delayed(packet, is_outgoing, 0, should_process); }
    void queue_packet_delayed(ENetPacket* packet, bool is_outgoing, float delay_ms, bool should_process = true) {
        m_secondary_packet_queue.enqueue({packet, is_outgoing, should_process, delay_ms}); }

private:
    server::Server* m_proxy_server;

    // used for threaded command execution.
    std::shared_ptr<BS::thread_pool> m_thread_pool;

    std::shared_ptr<ClientContext> m_ctx;

    // bool = is outgoing packet
    // why? to equalize the outgoing and incoming packet's priority ( I LOVE DEMOCRACY AND EQUAL RIGHT RAHHHHH obv not being held gunpoint at )
    moodycamel::ConcurrentQueue<PacketInfo> m_primary_packet_queue;
    moodycamel::ConcurrentQueue<PacketInfo> m_secondary_packet_queue;

    World m_curr_world;
    Player m_curr_player;
    command::CommandManager m_command_manager;
};
}
