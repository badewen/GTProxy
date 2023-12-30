#pragma once
#include <array>
#include <cstdint>
#include <cassert>
#include <memory>
#include "enet/enet.h"
#include <spdlog/spdlog.h>
#include <proton/Variant.h>

namespace packet {
enum ePacketType : std::uint32_t {
    NET_MESSAGE_UNKNOWN = 0,
    NET_MESSAGE_SERVER_HELLO,
    NET_MESSAGE_GENERIC_TEXT,
    NET_MESSAGE_GAME_MESSAGE,
    NET_MESSAGE_GAME_PACKET,
    NET_MESSAGE_ERROR,
    NET_MESSAGE_TRACK,
    NET_MESSAGE_CLIENT_LOG_REQUEST,
    NET_MESSAGE_CLIENT_LOG_RESPONSE,
    NET_MESSAGE_MAX
};

enum eTankPacketType : std::uint8_t {
    PACKET_STATE = 0,
    PACKET_CALL_FUNCTION,
    PACKET_UPDATE_STATUS,
    PACKET_TILE_CHANGE_REQUEST,
    PACKET_SEND_MAP_DATA,
    PACKET_SEND_TILE_UPDATE_DATA,
    PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
    PACKET_TILE_ACTIVATE_REQUEST,
    PACKET_TILE_APPLY_DAMAGE,
    PACKET_SEND_INVENTORY_STATE,
    PACKET_ITEM_ACTIVATE_REQUEST,
    PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
    PACKET_SEND_TILE_TREE_STATE,
    PACKET_MODIFY_ITEM_INVENTORY,
    PACKET_ITEM_CHANGE_OBJECT,
    PACKET_SEND_LOCK,
    PACKET_SEND_ITEM_DATABASE_DATA,
    PACKET_SEND_PARTICLE_EFFECT,
    PACKET_SET_ICON_STATE,
    PACKET_ITEM_EFFECT,
    PACKET_SET_CHARACTER_STATE,
    PACKET_PING_REPLY,
    PACKET_PING_REQUEST,
    PACKET_GOT_PUNCHED,
    PACKET_APP_CHECK_RESPONSE,
    PACKET_APP_INTEGRITY_FAIL,
    PACKET_DISCONNECT,
    PACKET_BATTLE_JOIN,
    PACKET_BATTLE_EVENT,
    PACKET_USE_DOOR,
    PACKET_SEND_PARENTAL,
    PACKET_GONE_FISHIN,
    PACKET_STEAM,
    PACKET_PET_BATTLE,
    PACKET_NPC,
    PACKET_SPECIAL,
    PACKET_SEND_PARTICLE_EFFECT_V2,
    PACKET_ACTIVE_ARROW_TO_ITEM,
    PACKET_SELECT_TILE_INDEX,
    PACKET_SEND_PLAYER_TRIBUTE_DATA,
    PACKET_FTUE_SET_ITEM_TO_QUICK_INVENTORY,
    PACKET_PVE_NPC,
    PACKET_PVP_CARD_BATTLE,
    PACKET_PVE_APPLY_PLAYER_DAMAGE,
    PACKET_PVE_NPC_POSITION_DAMAGE,
    PACKET_SET_EXTRA_MODS,
    PACKET_ON_STEP_ON_TILE_MOD,
    PACKET_MAX
};

enum eTankPacketFlag : std::uint32_t {
    PACKET_FLAG_NONE = 0,
    PACKET_FLAG_UNK = (1 << 1),
    PACKET_FLAG_RESET_VISUAL_STATE = (1 << 2),
    PACKET_FLAG_EXTENDED = (1 << 3),
    PACKET_FLAG_ROTATE_LEFT = (1 << 4),
    PACKET_FLAG_ON_SOLID = (1 << 5),
    PACKET_FLAG_ON_FIRE_DAMAGE = (1 << 6),
    PACKET_FLAG_ON_JUMP = (1 << 7),
    PACKET_FLAG_ON_KILLED = (1 << 8),
    PACKET_FLAG_ON_PUNCHED = (1 << 9),
    PACKET_FLAG_ON_PLACED = (1 << 10),
    PACKET_FLAG_ON_TILE_ACTION = (1 << 11),
    PACKET_FLAG_ON_GOT_PUNCHED = (1 << 12),
    PACKET_FLAG_ON_RESPAWNED = (1 << 13),
    PACKET_FLAG_ON_COLLECT_OBJECT = (1 << 14),
    PACKET_FLAG_ON_TRAMPOLINE = (1 << 15),
    PACKET_FLAG_ON_DAMAGE = (1 << 16),
    PACKET_FLAG_ON_SLIDE = (1 << 17),
    PACKET_FLAG_ON_WALL_HANG = (1 << 21),
    PACKET_FLAG_ON_ACID_DAMAGE = (1 << 26)
};

#pragma pack(push, 1)
struct GameUpdatePacket {
    eTankPacketType type;
    std::uint8_t pad[3];
    std::int32_t net_id;
    std::uint8_t pad_2[4];

    union {
        eTankPacketFlag value;
        struct {
            std::uint32_t bNone : 1;
            std::uint32_t bUnk : 1;
            std::uint32_t bResetVisualState : 1;
            std::uint32_t bExtended : 1;
            std::uint32_t bRotateLeft : 1;
            std::uint32_t bOnSolid : 1;
            std::uint32_t bOnFireDamage : 1;
            std::uint32_t bOnJump : 1;
            std::uint32_t bOnKilled : 1;
            std::uint32_t bOnPunched : 1;
            std::uint32_t bOnPlaced : 1;
            std::uint32_t bOnTileAction : 1;
            std::uint32_t bOnGotPunched : 1;
            std::uint32_t bOnRespawned : 1;
            std::uint32_t bOnCollectObject : 1;
            std::uint32_t bOnTrampoline : 1;
            std::uint32_t bOnDamage : 1;
            std::uint32_t bOnSlide : 1;
            std::uint32_t pad_1 : 3;
            std::uint32_t bOnWallHang : 1;
            std::uint32_t pad_2 : 3;
            std::uint32_t bOnAcidDamage : 1;
            std::uint32_t pad_3 : 6;
        };
    } flags;

    std::uint8_t pad_3[4];

    std::uint32_t value; // usually item id
    float player_x, player_y;
    float x_velocity, y_velocity;

    std::uint8_t  pad_4[4];

    uint32_t tile_x, tile_y; // target tile coordinate.

    uint32_t extended_data_length;
};
static_assert((sizeof(GameUpdatePacket) == 56) && "Invalid GameUpdatePacket size.");
#pragma pack(pop)



inline ePacketType get_packet_type(const ENetPacket* packet)
{
    if (packet->dataLength > 3) {
        return static_cast<ePacketType>(*packet->data);
    }

    spdlog::error("Bad packet length, ignoring message");
    return ePacketType::NET_MESSAGE_UNKNOWN;
}

inline std::string get_text(const ENetPacket* packet)
{
    std::vector<char> temp (packet->data + 4, packet->data + packet->dataLength - 1);

    return { temp.begin(), temp.end() };
}

inline GameUpdatePacket* get_tank_packet(const ENetPacket* packet)
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

inline std::vector<uint8_t> get_extended_data(GameUpdatePacket* game_update_packet)
{
    if (!game_update_packet->flags.bExtended) {
        return {};
    }

    return { (uint8_t*)(((uintptr_t)game_update_packet) + sizeof(GameUpdatePacket)),
             (uint8_t*)(((uintptr_t)game_update_packet) + sizeof(GameUpdatePacket) + game_update_packet->extended_data_length - 1)
    };
}

inline VariantList get_varlist(GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != eTankPacketType::PACKET_CALL_FUNCTION) {
        return {};
    }

    VariantList varlist{};

    varlist.SerializeFromMem(get_extended_data(game_update_packet).data(), get_extended_data(game_update_packet).size());

    return varlist;
}

}
