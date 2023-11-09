#include <memory>
#include <utility>

#include "peer.h"
#include "../utils/random.h"

using namespace packet;

namespace player {
Peer::Peer(ENetPeer* peer)
    : m_peer{ peer }
{
}

Peer::~Peer()
{
}

ENetPacket* Peer::build_packet(eNetMessageType type, const std::vector<uint8_t>& data)
{
    std::vector<std::byte> packet_data(sizeof(type) + data.size());
    std::memcpy(packet_data.data(), &type, sizeof(type));
    std::memcpy(packet_data.data() + sizeof(eNetMessageType), data.data(), data.size());

    return enet_packet_create(packet_data.data(), packet_data.size(), ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket* Peer::build_packet(eNetMessageType type, const std::string& data) {
    return build_packet(type, std::vector<uint8_t>{ data.begin(), data.end() });
}

int Peer::send_packet(eNetMessageType type, const std::string& data)
{
    if (!m_peer) {
        return -1;
    }

    return send_packet_packet( build_packet(type, data) );
}

int Peer::send_packet_packet(ENetPacket* packet, bool destroy_packet)
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

ENetPacket* Peer::build_raw_packet(
        GameUpdatePacket* game_update_packet,
        eNetMessageType type,
        size_t length,
        uint8_t* extended_data,
        enet_uint32 flags
) {
    ENetPacket* packet;
    if (type == NET_MESSAGE_GAME_PACKET && game_update_packet->flags.bExtended) {
        packet = enet_packet_create(nullptr, length + game_update_packet->data_size + 5, flags);
        std::memcpy(packet->data, &type, sizeof(eNetMessageType));
        std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);
        std::memcpy(packet->data + length + sizeof(eNetMessageType), extended_data, game_update_packet->data_size);
    }
    else {
        packet = enet_packet_create(nullptr, length + 5, flags);
        std::memcpy(packet->data, &type, sizeof(eNetMessageType));
        std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);
    }

    return packet;
}

int
Peer::send_raw_packet(GameUpdatePacket *game_update_packet, eNetMessageType type, size_t length, uint8_t *extended_data,
                      enet_uint32 flags) {
    if (!m_peer) {
        return -1;
    }

    if (length > 1000000) {
        return -1;
    }

    return send_packet_packet(build_raw_packet(game_update_packet));
}

ENetPacket* Peer::build_variant_packet(VariantList variant_list, std::int32_t net_id, enet_uint32 flags)
{
    if (variant_list.Get(0).GetType() == eVariantType::TYPE_UNUSED) {
        return nullptr;
    }

    GameUpdatePacket game_update_packet{};
    game_update_packet.type = ePacketType::PACKET_CALL_FUNCTION;
    game_update_packet.net_id = net_id;
    game_update_packet.flags.bExtended = true;
    game_update_packet.data_size = 0;

    std::shared_ptr<std::uint8_t> data{ variant_list.SerializeToMem(&game_update_packet.data_size, nullptr) };
    variant_list.Reset();

    return build_raw_packet(&game_update_packet, eNetMessageType::NET_MESSAGE_GAME_PACKET, sizeof(GameUpdatePacket),
                            data.get(), flags);
}

int Peer::send_variant(VariantList variant_list, std::int32_t net_id, enet_uint32 flags)
{
    return send_packet_packet(build_variant_packet(std::move(variant_list), net_id, flags));
}
}