#pragma once

#include <threadpool/include/BS_thread_pool.hpp>

#include "../http/http.h"
#include "../config.h"
#include "../peer/peer.h"
#include "../client/client.h"
#include "../utils/event_manager.h"

namespace server {
// this class interface directly with the growtopia client.
class Server {
public:
    explicit Server() = default;
    ~Server() = default;

    bool start(Config conf);
    void stop();

    void on_connect(ENetPeer* peer);
    void on_receive(ENetPeer* peer, ENetPacket* packet);
    void on_disconnect(ENetPeer* peer);

    peer::Peer* get_gt_peer() { return m_gt_peer.get(); }
    client::Client* get_client() { return m_client.get(); }
    Config* get_config() { return &m_config; }

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
    void server_thread();
    void create_host(bool use_new_packet);

private:
    std::shared_ptr<BS::thread_pool> m_thread_pool;
    // refers to the client that is interfacing directly with the growtopia server.
    std::unique_ptr<client::Client> m_client;
    // gt client refers to the actual gt client that is connected to the server
    std::shared_ptr<peer::Peer> m_gt_peer;

    bool m_running;

    Config m_config;
    ENetHost* m_enet_host;

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
