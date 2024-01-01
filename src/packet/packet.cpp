#include "packet.h"

using namespace packet;

ePacketType packet::get_packet_type(const ENetPacket* packet)
{
    if (packet->dataLength > 3) {
        return static_cast<ePacketType>(*packet->data);
    }

    spdlog::error("Bad packet length, ignoring message");
    return ePacketType::NET_MESSAGE_UNKNOWN;
}

std::string packet::get_text(const ENetPacket* packet)
{
    std::vector<char> temp (packet->data + 4, packet->data + packet->dataLength - 1);

    return { temp.begin(), temp.end() };
}

GameUpdatePacket* packet::get_tank_packet(const ENetPacket* packet)
{
    if (packet->dataLength < sizeof(GameUpdatePacket)) {
        return nullptr;
    }

    auto game_update_packet{ reinterpret_cast<GameUpdatePacket*>(packet->data + 4) };
    if (!game_update_packet->flags.bExtended) {
        return game_update_packet;
    }

    if (packet->dataLength < game_update_packet->extended_data_length + sizeof(GameUpdatePacket)) {
        spdlog::error("Packet too small for extended packet to be valid");
        return nullptr;
    }

    return game_update_packet;
}

std::vector<uint8_t> packet::get_extended_data(GameUpdatePacket* game_update_packet)
{
    if (!game_update_packet->flags.bExtended) {
        return {};
    }

    return { (uint8_t*)(((uintptr_t)game_update_packet) + sizeof(GameUpdatePacket)),
             (uint8_t*)(((uintptr_t)game_update_packet) + sizeof(GameUpdatePacket) + game_update_packet->extended_data_length - 1)
    };
}

VariantList packet::get_varlist(GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != eTankPacketType::PACKET_CALL_FUNCTION) {
        return {};
    }

    VariantList varlist{};

    varlist.SerializeFromMem(get_extended_data(game_update_packet).data(), get_extended_data(game_update_packet).size());

    return varlist;
}

ENetPacket* packet::create_packet(packet::ePacketType type, const std::vector<uint8_t>& data)
{
    std::vector<std::byte> packet_data(sizeof(type) + data.size());
    std::memcpy(packet_data.data(), &type, sizeof(ePacketType));
    std::memcpy(packet_data.data() + sizeof(ePacketType), data.data(), data.size());

    return enet_packet_create(packet_data.data(), packet_data.size(), ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket* packet::create_packet(packet::ePacketType type, const std::string& data) {
    std::vector<uint8_t> temp { data.data(), data.data() + data.size() - 1};

    temp.push_back('\0');

    return create_packet(type, temp);
}

ENetPacket* packet::create_raw_packet(
        packet::GameUpdatePacket *game_update_packet,
        packet::ePacketType type,
        std::size_t length,
        uint8_t* extended_data,
        enet_uint32 flags
) {
    ENetPacket* packet;
    if (type == NET_MESSAGE_GAME_PACKET && game_update_packet->flags.bExtended) {
        packet = enet_packet_create(nullptr, length + game_update_packet->extended_data_length + 5, flags);
        std::memcpy(packet->data, &type, sizeof(ePacketType));
        std::memcpy(packet->data + sizeof(ePacketType), game_update_packet, length);
        std::memcpy(packet->data + length + sizeof(ePacketType), extended_data, game_update_packet->extended_data_length);
    }
    else {
        packet = enet_packet_create(nullptr, length + 5, flags);
        std::memcpy(packet->data, &type, sizeof(ePacketType));
        std::memcpy(packet->data + sizeof(ePacketType), game_update_packet, length);
    }

    return packet;
}

ENetPacket* packet::create_varlist_packet(VariantList variant_list, std::int32_t net_id, enet_uint32 flags)
{
    if (variant_list.Get(0).GetType() == eVariantType::TYPE_UNUSED) {
        return nullptr;
    }

    GameUpdatePacket game_update_packet{};
    game_update_packet.type = eTankPacketType::PACKET_CALL_FUNCTION;
    game_update_packet.net_id = net_id;
    game_update_packet.flags.bExtended = true;
    game_update_packet.extended_data_length = 0;

    std::uint8_t* serialized_varlist = variant_list.SerializeToMem(&game_update_packet.extended_data_length, nullptr);
    variant_list.Reset();

    ENetPacket* created_packet = create_raw_packet(
            &game_update_packet,
            ePacketType::NET_MESSAGE_GAME_PACKET,
            sizeof(GameUpdatePacket),
            serialized_varlist, flags
    );

    delete[] serialized_varlist;

    return created_packet;
}
