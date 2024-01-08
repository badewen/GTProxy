#pragma once

#include "threadpool/include/BS_thread_pool.hpp"
#include "concurrentqueue/concurrentqueue.h"

#include "../http/http.h"
#include "../../config.h"
#include "../../peer/peer.h"
#include "../../utils/event_manager.h"
#include "../../utils/timer.h"
#include "proton/Variant.h"

namespace server {
    class Server;
}

namespace client {
// this class interface directly with the growtopia client.
class Client {
public:
    explicit Client() = default;
    ~Client() = default;

    bool init(server::Server* proxy_server, std::shared_ptr<BS::thread_pool> thread_pool);
    bool start();
    bool connect(ENetAddress addr, bool use_new_packet);
    void stop();

    void on_connect(ENetPeer* peer);
    void on_receive(ENetPeer* peer, ENetPacket* packet);
    void on_disconnect(ENetPeer* peer);

    std::shared_ptr<peer::Peer> get_server_peer() { return m_gt_server_peer; }
    server::Server* get_server() { return m_proxy_server; }

    void incoming_packet_events_invoke(ENetPacket* packet, bool* forward_packet);

    void send_to_gt_server(ENetPacket* packet, bool invoke_event);
    void send_to_gt_server_delayed(ENetPacket* packet, float delay_ms, bool invoke_event);

    void print_packet_info_incoming(ENetPacket* packet);

    template<typename FuncType, typename ... FuncParams>
    inline void add_on_connect_callback(std::string id, FuncType func, FuncParams ... func_data) {
        m_on_connect_callbacks.Register(id, func, std::forward<FuncParams>(func_data)...);
    }
    inline void remove_on_connect_callback(const std::string& id) { m_on_connect_callbacks.Remove(id); }

    template<typename FuncType, typename ... FuncParams>
    inline void add_on_disconnect_callback(std::string id, FuncType func, FuncParams... func_data) {
        m_on_disconnect_callbacks.Register(id, func, std::forward<FuncParams>(func_data)...);
    }
    inline void remove_on_disconnect_callback(const std::string& id) { m_on_disconnect_callbacks.Remove(id); }

    template<typename FuncType, typename ... FuncParams>
    inline void add_on_incoming_packet_callback(std::string id, FuncType func, FuncParams... func_data) {
        m_on_incoming_packet.Register(id, func, std::forward<FuncParams>(func_data)...);
    }
    inline void remove_on_incoming_packet_callback(const std::string& id) { m_on_incoming_packet.Remove(id); }

    template<typename FuncType, typename ... FuncParams>
    inline void add_on_incoming_tank_packet_callback(std::string id, FuncType func, FuncParams... func_data) {
        m_on_incoming_tank_packet.Register(id, func, std::forward<FuncParams>(func_data)...);
    }
    inline void remove_on_incoming_tank_packet_callback(const std::string& id) { m_on_incoming_tank_packet.Remove(id); }

    template<typename FuncType, typename ... FuncParams>
    inline void add_on_incoming_varlist_packet_callback(std::string id, FuncType func, FuncParams... func_data) {
        m_on_incoming_varlist_packet.Register(id, func, std::forward<FuncParams>(func_data)...);
    }
    inline void remove_on_incoming_varlist_packet_callback(const std::string& id) { m_on_incoming_tank_packet.Remove(id); }

private:
    struct packetInfoStruct {
        utils::Timer Delay;
        ENetPacket* Packet;
        bool InvokeEvents;
    };

private:
    void client_thread();
    void process_delayed_packet_thread();

    void create_host();

private:
    bool m_running {};

    std::shared_ptr<BS::thread_pool> m_thread_pool;
    // refers to the client that is interfacing directly with the growtopia server.
    server::Server* m_proxy_server{};
    // gt client refers to the actual gt client that is connected to the server
    std::shared_ptr<peer::Peer> m_gt_server_peer;
    ENetHost* m_enet_host {};

    moodycamel::ConcurrentQueue<packetInfoStruct> m_delayed_packet_primary_queue {};
    moodycamel::ConcurrentQueue<packetInfoStruct> m_delayed_packet_secondary_queue {};

    utils::EventManager<std::shared_ptr<peer::Peer /* gt_server_peer */>> m_on_connect_callbacks {};
    utils::EventManager<std::shared_ptr<peer::Peer /* gt_server_peer */>> m_on_disconnect_callbacks {};

    utils::EventManager<ENetPacket* /* packet */,
            std::shared_ptr<peer::Peer> /* gt_server_peer */,
            bool* /* forward packet */> m_on_incoming_packet;

    utils::EventManager<packet::GameUpdatePacket* /*packet*/,
            std::shared_ptr<peer::Peer> /* gt_server_peer */,
            bool* /* forward packet */> m_on_incoming_tank_packet;

    utils::EventManager<VariantList* /*packet*/,
            int32_t, /* net_id */
            std::shared_ptr<peer::Peer> /* gt_server_peer */,
            bool* /* forward packet */> m_on_incoming_varlist_packet;
};
}
