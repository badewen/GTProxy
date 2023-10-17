#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include <enet/enet.h>
#include "../peer/peer.h"

namespace enet_wrapper {
class ENetClient {
public:
    ENetClient();
    ~ENetClient();

    bool create_host(std::size_t peer_count, enet_uint8 using_new_packet);
    void destroy_host();
    bool connect(const std::string& host, enet_uint16 port, enet_uint32 connectID);

    void start_service();
    void service_thread();

    virtual void on_connect(ENetPeer* peer) = 0;
    virtual void on_service_loop() = 0;
    virtual void on_receive(ENetPeer* peer, ENetPacket* packet) = 0;
    virtual void on_disconnect(ENetPeer* peer) = 0;

    player::Peer* to_peer() { return m_peer_wrapper; }

protected:
    ENetHost* m_host;
    ENetPeer* m_peer;
    // just a convenient way.
    player::Peer* m_peer_wrapper;
    std::thread m_service_thread;
    std::atomic<bool> m_running;
};
}