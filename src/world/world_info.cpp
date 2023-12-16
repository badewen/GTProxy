#include "world_info.h"

#include <utility>

using namespace world;

bool WorldInfo::serialize(std::vector<uint8_t> raw_data) {
    m_raw_data = std::move(raw_data);
    m_curr_it = m_raw_data.begin();

    std::advance(m_curr_it, 6);

    Name = get_str();
    Width = get_int<uint32_t>();
    Height = get_int<uint32_t>();
    TotalTiles = get_int<uint32_t>();

    std::advance(m_curr_it, 5);

    Tiles.reserve(TotalTiles);

    for (uint32_t i = 0; i < TotalTiles; i++) {
        Tile temp {};
        if (!get_next_tile(temp)) {
            return false;
        }
        Tiles.push_back(temp);
    }

    parse_item_drops();

    BaseWeather = get_int<uint16_t>();
    std::advance(m_curr_it, 2);
    CurrentWeather = get_int<uint16_t>();
    std::advance(m_curr_it, 6);
    return true;
}

bool WorldInfo::get_next_tile(Tile& out) {
    out.Fg = get_int<uint16_t>();
    out.Bg = get_int<uint16_t>();
    out.ParentTileIndex = get_int<uint16_t>();
    out.Flags.Value = get_int<TileFlag>();
    out.ExtraTileDataType = eExtraTileDataType::None;

    if (out.Flags.bLocked) {
        out.LockIndex = get_int<uint16_t>();
    }

    if (out.Flags.bHasTileExtra) {
        out.ExtraTileDataType = get_int<eExtraTileDataType>();
    }

    return parse_tile_extra(out);
}

bool WorldInfo::parse_tile_extra(Tile& tile) {
    switch (tile.ExtraTileDataType) {
        case None: {
			break;
		}
        case Door: {
			TileDoorExtra extra_data {};

            extra_data.Label = get_str();
            extra_data.Unk1 = get_int<uint8_t>();

            *tile.ExtraTileData.DoorExtra = extra_data;
            break;
		}
        
        case Sign: {
			TileSignExtra extra_data {};

            extra_data.Label = get_str();

            std::advance(m_curr_it, 4); // end mark

            *tile.ExtraTileData.SignExtra = extra_data;
            break;
		}
            
        case Lock: {
			TileLockExtra extra_data {};

            extra_data.Flag = get_int<uint8_t>();
            extra_data.OwnerId = get_int<uint32_t>();
            extra_data.AccessList = get_list<uint32_t, uint32_t>();

            if (tile.Fg != 202 ||
                tile.Fg != 204 ||
                tile.Fg != 206 ||
                tile.Fg != 4994
            ) {
                extra_data.MinimumLevel = get_int<uint8_t>();
                copy_raw_byte_array(&extra_data.UnkArr, 7);
            }

            *tile.ExtraTileData.LockExtra = extra_data;
            break;
		}
            
        case Seed: {
			TileSeedExtra extra_data {};

            extra_data.TimePassed = get_int<uint32_t>();
            extra_data.FruitCount = get_int<uint8_t>();

            *tile.ExtraTileData.SeedExtra = extra_data;
            break;
		}
            
        case Dice: {
			TileDiceExtra extra_data {};

            extra_data.Pattern = get_int<uint8_t>();

            *tile.ExtraTileData.DiceExtra = extra_data;
            break;
		}
            
        case Provider: {
			TileProviderExtra extra_data {};

            extra_data.TimePassed = get_int<uint32_t>();

            *tile.ExtraTileData.ProviderExtra = extra_data;
            break;
		}
            
        case AchievementBlock: {
			TileAchievementBlockExtra extra_data {};

            extra_data.UserId = get_int<uint32_t>();
            extra_data.Achievement = get_int<uint8_t>();

            *tile.ExtraTileData.AchievementBlockExtra = extra_data;
            break;
		}
            
        case HeartMonitor: {
			TileHeartMonitorExtra extra_data {};

            extra_data.UserId = get_int<uint32_t>();
            extra_data.Name = get_str();

            *tile.ExtraTileData.HeartMonitorExtra = extra_data;
            break;
		}
            
        case Mannequin: {
			TileMannequinExtra extra_data {};

            extra_data.Label = get_str();
            extra_data.Unk1 = get_int<uint8_t>();
            extra_data.Unk2 = get_int<uint16_t>();
            extra_data.Unk3 = get_int<uint16_t>();
            extra_data.Hat = get_int<uint16_t>();
            extra_data.Shirt = get_int<uint16_t>();
            extra_data.Pants = get_int<uint16_t>();
            extra_data.Boots = get_int<uint16_t>();
            extra_data.Face = get_int<uint16_t>();
            extra_data.Hand = get_int<uint16_t>();
            extra_data.Back = get_int<uint16_t>();
            extra_data.Hair = get_int<uint16_t>();
            extra_data.Neck = get_int<uint16_t>();

            *tile.ExtraTileData.MannequinExtra = extra_data;
            break;
		}
            
        case GameGrave: {
			TileGameGraveExtra extra_data {};

            extra_data.Team = get_int<uint8_t>();

            *tile.ExtraTileData.GameGraveExtra = extra_data;
            break;
		}
            
        case GameGenerator: {
			TileGameGeneratorExtra extra_data {};

            *tile.ExtraTileData.GameGeneratorExtra = extra_data;
            break;
		}
            
        case Xenonite: {
			TileXenoniteExtra extra_data {};

            copy_raw_byte_array(&extra_data.Unk1, 5);

            *tile.ExtraTileData.XenoniteExtra = extra_data;
            break;
		}
            
        case PhoneBooth: {
			TilePhoneBoothExtra extra_data {};

            extra_data.Hat = get_int<uint16_t>();
            extra_data.Shirt = get_int<uint16_t>();
            extra_data.Pants = get_int<uint16_t>();
            extra_data.Shoes = get_int<uint16_t>();
            extra_data.Face = get_int<uint16_t>();
            extra_data.Hand = get_int<uint16_t>();
            extra_data.Back = get_int<uint16_t>();
            extra_data.Hair = get_int<uint16_t>();
            extra_data.Neck = get_int<uint16_t>();

            *tile.ExtraTileData.PhoneBoothExtra = extra_data;
            break;
		}
            
        case Spotlight: {
			TileSpotlightExtra extra_data {};

            *tile.ExtraTileData.SpotlightExtra = extra_data;
            break;
		}
            
        case DisplayBlock: {
			TileDisplayBlockExtra extra_data {};

            extra_data.ItemId = get_int<uint32_t>();

            *tile.ExtraTileData.DisplayBlockExtra = extra_data;
            break;
		}
            
        case VendingMachine: {
			TileVendingMachineExtra extra_data {};

            extra_data.ItemId = get_int<uint32_t>();
            extra_data.Price = get_int<int32_t>();

            *tile.ExtraTileData.VendingMachineExtra = extra_data;
            break;
		}
            
        case FishTankPort: {
			TileFishTankPortExtra extra_data {};

            extra_data.Flag = get_int<uint8_t>();
            for (uint32_t i = 0; i < get_int<uint32_t>()/2; i++) {
                extra_data.Fishes.push_back({
                    .FishItemId = get_int<uint32_t>(),
                    .Lbs = get_int<uint32_t>()
                });
            }

            *tile.ExtraTileData.FishTankPortExtra = extra_data;
            break;
		}
            
        case Forge: {
			TileForgeExtra extra_data {};

            extra_data.Temperature = get_int<uint32_t>();

            *tile.ExtraTileData.ForgeExtra = extra_data;
            break;
		}
            
        case GivingTree: {
			TileGivingTreeExtra extra_data {};

            extra_data.Harvested = get_int<uint8_t>();
            extra_data.Unk1 = get_int<uint32_t>();
            extra_data.DecorationPercent = get_int<uint8_t>();

            *tile.ExtraTileData.GivingTreeExtra = extra_data;
            break;
		}
            
        case SewingMachine: {
			TileSewingMachineExtra extra_data {};

            extra_data.BoltIdList = get_list<uint32_t, uint32_t>();

            *tile.ExtraTileData.SewingMachineExtra = extra_data;
            break;
		}
            
        case CountryFlag: {
			TileCountryFlagExtra extra_data {};

            extra_data.Country = get_str();

            *tile.ExtraTileData.CountryFlagExtra = extra_data;
            break;
		}
            
        case LobsterTrap: {
			TileLobsterTrapExtra extra_data {};

            *tile.ExtraTileData.LobsterTrapExtra = extra_data;
            break;
		}
            
        case PaintingEasel: {
			TilePaintingEaselExtra extra_data {};

            extra_data.ItemId = get_int<uint32_t>();
            extra_data.Label = get_str();

            *tile.ExtraTileData.PaintingEaselExtra = extra_data;
            break;
		}
            
        case WeatherMachine: {
			TileWeatherMachineExtra extra_data {};

            extra_data.Flag = get_int<uint32_t>();

            *tile.ExtraTileData.WeatherMachineExtra = extra_data;
            break;
		}
            
        case DataBedrock: {
			TileDataBedrockExtra extra_data {};

            copy_raw_byte_array(&extra_data.Unk1, 21);

            *tile.ExtraTileData.DataBedrockExtra = extra_data;
            break;
		}
            
        case Shelf: {
			TileShelfExtra extra_data {};

            extra_data.TopLeft = get_int<uint32_t>();
            extra_data.TopRight = get_int<uint32_t>();
            extra_data.BottomLeft = get_int<uint32_t>();
            extra_data.BottomRight = get_int<uint32_t>();

            *tile.ExtraTileData.ShelfExtra = extra_data;
            break;
		}
            
        case VipEntrance: {
			TileVipEntranceExtra extra_data {};

            extra_data.Unk1 = get_int<uint8_t>();
            extra_data.OwnerId = get_int<uint32_t>();
            extra_data.AllowedIdList = get_list<uint32_t, uint32_t>();

            *tile.ExtraTileData.VipEntranceExtra = extra_data;
            break;
		}
            
        case ChallengeTimer: {
			TileChallengeTimerExtra extra_data {};

            *tile.ExtraTileData.ChallengeTimerExtra = extra_data;
            break;
		}
            
        case Portrait: {
			TilePortraitExtra extra_data {};

            extra_data.Label = get_str();
            extra_data.Unk1 = get_int<uint32_t>();
            extra_data.Unk2 = get_int<uint32_t>();
            extra_data.Unk3 = get_int<uint32_t>();
            extra_data.Unk4 = get_int<uint32_t>();
            extra_data.Face = get_int<uint32_t>();
            extra_data.Hat = get_int<uint32_t>();
            extra_data.Hair = get_int<uint32_t>();
            extra_data.Unk5 = get_int<uint32_t>();
            extra_data.Unk6 = get_int<uint32_t>();

            *tile.ExtraTileData.PortraitExtra = extra_data;
            break;
		}
            
        case GuildWeatherMachine: {
			TileGuildWeatherMachineExtra extra_data {};

            extra_data.Unk1 = get_int<uint32_t>();
            extra_data.Gravity = get_int<uint32_t>();
            extra_data.Flag = get_int<uint8_t>();

            *tile.ExtraTileData.GuildWeatherMachineExtra = extra_data;
            break;
		}
            
        case DnaExtractor: {
			TileDnaExtractorExtra extra_data {};

            *tile.ExtraTileData.DnaExtractorExtra = extra_data;
            break;
		}
            
        case Howler: {
			TileHowlerExtra extra_data {};

            *tile.ExtraTileData.HowlerExtra = extra_data;
            break;
		}
            
        case StorageBlock: {
			TileStorageBlockExtra extra_data {};

            for (uint16_t i = 0; i < get_int<uint16_t>() / 13; i++) {
                std::advance(m_curr_it, 3);
                uint32_t item_id = get_int<uint32_t>();

                std::advance(m_curr_it, 2);
                uint32_t item_amount = get_int<uint32_t>();

                extra_data.Items.push_back({
                    .ItemId = item_id,
                    .ItemAmount = item_amount
                });
            }

            *tile.ExtraTileData.StorageBlockExtra = extra_data;
            break;
		}
            
        case CookingOven: {
			TileCookingOvenExtra extra_data {};

            *tile.ExtraTileData.CookingOvenExtra = extra_data;
            break;
		}
            
        case AudioRack: {
			TileAudioRackExtra extra_data {};

            extra_data.Note = get_str();
            extra_data.Volume = get_int<uint32_t>();

            *tile.ExtraTileData.AudioRackExtra = extra_data;
            break;
		}
            
        case AdventureBegin: {
			TileAdventureBeginsExtra extra_data {};

            *tile.ExtraTileData.AdventureBeginsExtra = extra_data;
            break;
		}
            
        case TombRobber: {
			TileTombRobberExtra extra_data {};

            *tile.ExtraTileData.TombRobberExtra = extra_data;
            break;
		}
            
        case TrainingPort: {
			TileTrainingPortExtra extra_data {};

            extra_data.FishLb = get_int<uint32_t>();
            extra_data.FishStatus = get_int<uint16_t>();
            extra_data.FishId = get_int<uint32_t>();
            extra_data.FishTotalExp = get_int<uint32_t>();
            copy_raw_byte_array(&extra_data.Unk1, 8);
            extra_data.FishLevel = get_int<uint32_t>();
            extra_data.Unk2 = get_int<uint32_t>();
            copy_raw_byte_array(&extra_data.Unk3, 5);

            *tile.ExtraTileData.TrainingPortExtra = extra_data;
            break;
		}
            
        case GuildItem: {
			TileGuildItemExtra extra_data {};

            copy_raw_byte_array(&extra_data.Unk1, 17);

            *tile.ExtraTileData.GuildItemExtra = extra_data;
            break;
		}
            
        case KrakenGalaticBlock: {
			TileKrakenBlockExtra extra_data {};

            extra_data.PatternIndex = get_int<uint8_t>();
            extra_data.Unk1 = get_int<uint32_t>();

            // this should be in the right order.
            extra_data.R = get_int<uint8_t>();
            extra_data.G = get_int<uint8_t>();
            extra_data.B = get_int<uint8_t>();

            *tile.ExtraTileData.KrakenBlockExtra = extra_data;
            break;
		}
            
        case FriendsEntrance: {
			TileFriendsEntranceExtra extra_data {};

            extra_data.OwnerUserId = get_int<uint32_t>();
            extra_data.Unk1 = get_int<uint16_t>();
            extra_data.AllowedUserList = get_list<uint16_t, uint32_t>();

            *tile.ExtraTileData.FriendEntranceExtra = extra_data;
            break;
		}

        default: {
            // not handled. return false
            return false;
        }
    }
    return true;
}

void WorldInfo::parse_item_drops() {
    // not a useful data for now.
    std::advance(m_curr_it, 12);

    uint32_t item_length = get_int<uint32_t>();
    std::advance(m_curr_it, 4);

    for (uint32_t i = 0; i < item_length; i++) {
        ItemDrops.push_back({
            .ItemId = get_int<uint16_t>(),
            .X = get_int<float>(),
            .Y = get_int<float>(),
            .Amount = get_int<uint8_t>(),
            .Flag = get_int<uint8_t>(),
            .Uid = get_int<uint32_t>()
        });
    }
}

void WorldInfo::reset() {
    Name = "";
    Height = 0;
    Width = 0;
    TotalTiles = 0;
    BaseWeather = 0;
    CurrentWeather = 0;
    Tiles.clear();
    ItemDrops.clear();
    m_raw_data.clear();
    m_curr_it = m_raw_data.begin();
}


