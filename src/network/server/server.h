#pragma once

#include "threadpool/include/BS_thread_pool.hpp"

#include "../http/http.h"
#include "../../config.h"
#include "../../peer/peer.h"
#include "../client/client.h"
#include "../../utils/event_manager.h"
#include "../../module/module_manager.h"

namespace server {
// this class interface directly with the growtopia client and handles outgoing packets.
// just a barebone server implementation that is only capable of receiving growtopia packets
// and forward then invoking the packet events callbacks
class Server {
public:
    explicit Server() = default;
    ~Server() = default;

    bool init(Config conf);
    bool start();
    void stop();

    void on_connect(ENetPeer* peer);
    void on_receive(ENetPeer* peer, ENetPacket* packet);
    void on_disconnect(ENetPeer* peer);

    peer::Peer* get_gt_peer() { return m_gt_peer.get(); }
    client::Client* get_client() { return m_client.get(); }
    Config* get_config() { return &m_config; }

    void outgoing_packet_events_invoke(ENetPacket* packet, bool* forward_packet);

    void send_to_gt_client(ENetPacket* packet, bool invoke_event = true);
    void send_to_gt_server_delayed(ENetPacket* packet, float delay_ms, bool invoke_event = true);

    void print_packet_info_outgoing(ENetPacket* packet);

    template<typename FuncType>
    inline void add_on_connect_callback(std::string id, FuncType func) { m_on_connect_callbacks.Register(id, func); }
    inline void remove_on_connect_callback(const std::string& id) { m_on_connect_callbacks.Remove(id); }

    template<typename FuncType>
    inline void add_on_disconnect_callback(std::string id, FuncType func) { m_on_disconnect_callbacks.Register(id, func); }
    inline void remove_on_disconnect_callback(const std::string& id) { m_on_disconnect_callbacks.Remove(id); }

    template<typename FuncType>
    inline void add_on_outgoing_packet_callback(std::string id, FuncType func) { m_on_outgoing_packet.Register(id, func); }
    inline void remove_on_outgoing_packet_callback(const std::string& id) { m_on_outgoing_packet.Remove(id); }

    template<typename FuncType>
    inline void add_on_outgoing_tank_packet_callback(std::string id, FuncType func) { m_on_outgoing_tank_packet.Register(id, func); }
    inline void remove_on_outgoing_tank_packet_callback(const std::string& id) { m_on_outgoing_tank_packet.Remove(id); }

private:
    struct packetInfoStruct {
        utils::Timer Delay;
        ENetPacket* Packet;
        bool InvokeEvents;
    };

private:
    void server_thread();
    void process_delayed_packet_thread();

    void create_host();

private:
    std::shared_ptr<BS::thread_pool> m_thread_pool;
    // refers to the client that is interfacing directly with the growtopia server.
    std::unique_ptr<client::Client> m_client;
    // gt client refers to the actual gt client that is connected to the server
    std::shared_ptr<peer::Peer> m_gt_peer;

    bool m_running {};

    Config m_config;
    ENetHost* m_enet_host {};

    moodycamel::ConcurrentQueue<packetInfoStruct> m_delayed_packet_primary_queue;
    moodycamel::ConcurrentQueue<packetInfoStruct> m_delayed_packet_secondary_queue;

    module::ModuleManager m_module_manager {};

    utils::EventManager<std::shared_ptr<peer::Peer>> m_on_connect_callbacks;
    utils::EventManager<std::shared_ptr<peer::Peer>> m_on_disconnect_callbacks;

    utils::EventManager<ENetPacket* /* packet */,
                        std::shared_ptr<peer::Peer> /* peer */,
                        bool* /* forward packet */> m_on_outgoing_packet;

    utils::EventManager<packet::GameUpdatePacket* /*packet*/,
                        std::shared_ptr<peer::Peer> /* gt_peer*/,
                        bool* /* forward packet */> m_on_outgoing_tank_packet;
};
}
