#pragma once

#include <cstdint>

#include "extra_tile_data.h"

// Credit : https://github.com/RebillionXX/GrowTiles/tree/main/tile_data

namespace world {

enum TileFlag : uint16_t {
    TILEFLAG_TILEEXTRA = 1 << 0,
    TILEFLAG_LOCKED = 1 << 1,
    TILEFLAG_SEED = 1 << 4,
    TILEFLAG_FLIPPED = 1 << 5,
    TILEFLAG_OPEN = 1 << 6,
    TILEFLAG_PUBLIC = 1 << 7,
    TILEFLAG_SILENCED = 1 << 9,
    TILEFLAG_WATER = 1 << 10,
    TILEFLAG_FIRE = 1 << 12,
    TILEFLAG_RED = 1 << 13,
    TILEFLAG_BLUE = 1 << 14,
    TILEFLAG_GREEN = 1 << 15,
};

class Tile {
public:
    Tile() {
        ExtraTileData._struct_buff = m_extra_tile_data_buff;
    }

public:
    uint16_t Fg{}, Bg{};
    // for fish tank, lock, etc.
    uint16_t ParentTileIndex{};

    union {
        TileFlag Value;
        struct {
            uint16_t bHasTileExtra : 1;
            uint16_t bLocked : 1;
            uint16_t bUnk1 : 1;
            uint16_t bUnk2 : 1;
            uint16_t bSeed : 1;
            uint16_t bFlipped : 1;
            uint16_t bOpen : 1;
            uint16_t bPublic : 1;
            uint16_t bUnk3 : 1;
            uint16_t bSilenced : 1;
            uint16_t bWater : 1;
            uint16_t bUnk4 : 1;
            uint16_t bFire : 1;
            uint16_t bRed : 1;
            uint16_t bBlue : 1;
            uint16_t bGreen : 1;
        };
    } Flags{};

    // if bLocked is set, this contains the index to the locking block.
    uint16_t LockIndex{};

    // if bTileExtra is set.
    world::eExtraTileDataType ExtraTileDataType;

    union {
        // c++ constructor and destructor non-sense.
        uint8_t* _struct_buff;
        world::TileDoorExtra* DoorExtra;
        world::TileSignExtra* SignExtra;
        world::TileLockExtra* LockExtra;
        world::TileSeedExtra* SeedExtra;
        world::TileDiceExtra* DiceExtra;
        world::TileProviderExtra* ProviderExtra;
        world::TileAchievementBlockExtra* AchievementBlockExtra;
        world::TileHeartMonitorExtra* HeartMonitorExtra;
        world::TileMannequinExtra* MannequinExtra;
        world::TileGameGraveExtra* GameGraveExtra;
        world::TileGameGeneratorExtra* GameGeneratorExtra;
        world::TileXenoniteExtra* XenoniteExtra;
        world::TilePhoneBoothExtra* PhoneBoothExtra;
        world::TileSpotlightExtra* SpotlightExtra;
        world::TileDisplayBlockExtra* DisplayBlockExtra;
        world::TileVendingMachineExtra* VendingMachineExtra;
        world::TileFishTankPortExtra* FishTankPortExtra;
        world::TileForgeExtra* ForgeExtra;
        world::TileGivingTreeExtra* GivingTreeExtra;
        world::TileSewingMachineExtra* SewingMachineExtra;
        world::TileCountryFlagExtra* CountryFlagExtra;
        world::TileLobsterTrapExtra* LobsterTrapExtra;
        world::TilePaintingEaselExtra* PaintingEaselExtra;
        world::TileWeatherMachineExtra* WeatherMachineExtra;
        world::TileDataBedrockExtra* DataBedrockExtra;
        world::TileShelfExtra* ShelfExtra;
        world::TileVipEntranceExtra* VipEntranceExtra;
        world::TileChallengeTimerExtra* ChallengeTimerExtra;
        world::TilePortraitExtra* PortraitExtra;
        world::TileGuildWeatherMachineExtra* GuildWeatherMachineExtra;
        world::TileDnaExtractorExtra* DnaExtractorExtra;
        world::TileHowlerExtra* HowlerExtra;
        world::TileStorageBlockExtra* StorageBlockExtra;
        world::TileCookingOvenExtra* CookingOvenExtra;
        world::TileAudioRackExtra* AudioRackExtra;
        world::TileAdventureBeginsExtra* AdventureBeginsExtra;
        world::TileTombRobberExtra* TombRobberExtra;
        world::TileTrainingPortExtra* TrainingPortExtra;
        world::TileGuildItemExtra* GuildItemExtra;
        world::TileKrakenBlockExtra* KrakenBlockExtra;
        world::TileFriendsEntranceExtra* FriendEntranceExtra;
    } ExtraTileData{};
private:
    // large enough to contain all of the extra tile structs
    uint8_t m_extra_tile_data_buff[64]{};
};

}
