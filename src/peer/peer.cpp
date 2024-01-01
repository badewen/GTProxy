#include <memory>
#include <utility>

#include "peer.h"
#include "../utils/random.h"

using namespace packet;
using namespace peer;

Peer::Peer(ENetPeer* peer)
    : m_peer{ peer }
{
}

Peer::~Peer()
{
}

int Peer::send_packet(ePacketType type, const std::string& data)
{
    if (!m_peer) {
        return -1;
    }

    return send_enet_packet(create_packet(type, data) );
}

int Peer::send_packet(ePacketType type, const std::vector<uint8_t>& data)
{
    if (!m_peer) {
        return -1;
    }

    return send_enet_packet(create_packet(type, data) );
}

int Peer::send_enet_packet(ENetPacket* packet, bool destroy_packet)
{
    if (!m_peer) {
        return -1;
    }
    send_lock.lock();
    int ret = enet_peer_send(m_peer, 0, packet);
    send_lock.unlock();

    if (destroy_packet) {
        enet_packet_destroy(packet);
    }

    return ret;
}

int
Peer::send_raw_packet(GameUpdatePacket *game_update_packet, ePacketType type, size_t length, uint8_t *extended_data,
                      enet_uint32 flags) {
    if (!m_peer) {
        return -1;
    }

    if (length > 1000000) {
        return -1;
    }

    return send_enet_packet(create_raw_packet(game_update_packet));
}

int Peer::send_variant(VariantList variant_list, std::int32_t net_id, enet_uint32 flags)
{
    return send_enet_packet(create_varlist_packet(std::move(variant_list), net_id, flags));
}