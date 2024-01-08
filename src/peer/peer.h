#pragma once
#include <string>
#include "enet/include/enet/enet.h"
#include "proton/Variant.h"
#include <mutex>
#include "../packet/packet.h"

namespace peer {
class Peer {
public:
    explicit Peer(ENetPeer* peer);
    ~Peer();

    int send_packet(packet::ePacketType type, const std::vector<uint8_t> &data);
    int send_packet(packet::ePacketType type, const std::string& data);
    int send_enet_packet(ENetPacket* packet, bool destroy_packet = true);
    int send_raw_packet(packet::GameUpdatePacket *game_update_packet,
                    packet::ePacketType type = packet::ePacketType::NET_MESSAGE_GAME_PACKET,
                    std::size_t length = sizeof(packet::GameUpdatePacket),
                    std::uint8_t* extended_data = nullptr,
                    enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

    int send_variant(VariantList variant_list, std::int32_t net_id = -1, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

public:
    // should be pretty bulletproof check now
    [[nodiscard]] bool is_connected() const { return m_peer->state == ENET_PEER_STATE_CONNECTED; }
    enet_uint32 get_connect_id() const { return m_peer->connectID; }
    void disconnect() { enet_peer_disconnect(m_peer, 0); }
    void disconnect_now() {  enet_peer_disconnect_now(m_peer, 0); }
    ENetPeer* get_raw_peer() const { return m_peer; }

private:
    ENetPeer* m_peer;
    std::mutex send_lock;

};
}
