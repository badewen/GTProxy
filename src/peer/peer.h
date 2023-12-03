#pragma once
#include <string>
#include "enet/include/enet/enet.h"
#include "util/Variant.h"
#include <mutex>
#include "../network/packet.h"

namespace player {
class Peer {
public:
    explicit Peer(ENetPeer* peer);
    ~Peer();

    static ENetPacket* build_packet(packet::PacketType type, const std::vector<uint8_t>& data);
    static ENetPacket* build_packet(packet::PacketType type, const std::string& data);
    static ENetPacket* build_raw_packet(packet::GameUpdatePacket *game_update_packet,
                                        packet::PacketType type = packet::PacketType::NET_MESSAGE_GAME_PACKET,
                                        std::size_t length = sizeof(packet::GameUpdatePacket),
                                        std::uint8_t* extended_data = nullptr,
                                        enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
    static ENetPacket* build_variant_packet(VariantList variant_list, std::int32_t net_id, enet_uint32 flags);

    int send_packet(packet::PacketType type, const std::vector<uint8_t> &data);
    int send_packet(packet::PacketType type, const std::string& data);
    int send_packet_packet(ENetPacket* packet, bool destroy_packet = false);
    int send_raw_packet(packet::GameUpdatePacket *game_update_packet,
                    packet::PacketType type = packet::PacketType::NET_MESSAGE_GAME_PACKET,
                    std::size_t length = sizeof(packet::GameUpdatePacket),
                    std::uint8_t* extended_data = nullptr,
                    enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

    int send_variant(VariantList variant_list, std::int32_t net_id = -1, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

public:
    [[nodiscard]] bool is_connected() const { return m_peer->state == ENET_PEER_STATE_CONNECTED; }
    enet_uint32 get_connect_id() const { return m_peer->connectID; }
    void disconnect() const { m_peer ? enet_peer_disconnect(m_peer, 0) : (void)0; }
    void disconnect_now() const { m_peer ? enet_peer_disconnect_now(m_peer, 0) : (void)0; }
    ENetPeer* get_raw_peer() const { return m_peer; }

private:
    ENetPeer* m_peer;
    std::mutex send_lock;

};
}
