#pragma once

// Port of https://github.com/badewen/Growtopia-Things/blob/main/parsers/world_parser.py

#include <vector>
#include <string>
#include <type_traits>

#include "world_tile.h"

namespace world {

class WorldInfo {
public:
    WorldInfo() = default;

    // if failed, the Tiles and ItemDrops member is null.
    bool serialize(std::vector<uint8_t> raw_data);

    std::vector<uint8_t> get_raw_data() { return m_raw_data; }

    void reset();

public:
    struct DroppedItemInfo {
        uint16_t ItemId;
        float X, Y;
        uint8_t Amount;
        uint8_t Flag;
        uint32_t Uid;
    };

public:
    std::string Name {};
    uint32_t Width {}, Height {}, TotalTiles {};
    std::vector<Tile> Tiles {};
    std::vector<DroppedItemInfo> ItemDrops {};
    uint16_t BaseWeather {}, CurrentWeather {};

private:
// primitive getter
    template <typename T>
    T get_int() {
        // convert array of byte into normal integer
        T ret = *((T*)&(*m_curr_it));
        std::advance(m_curr_it, sizeof(T));
        return ret;
    }

    std::vector<uint8_t> get_byte_array(size_t sz) {
        std::vector<uint8_t> ret {};

        for (size_t i = 0; i < sz; i++) {
            ret.push_back(get_int<uint8_t>());
        }

        return ret;
    }

    void copy_raw_byte_array(void* dest, size_t sz) {
        memcpy(dest, get_byte_array(sz).data(), sz);
    }

// getter built on top of primitive getter

    // ElmType = the array/list element type
    // LenType = the array/list size in byte. Because the list length is specified at the beginning of the array/list.
    template <typename LenType, typename ElmType>
    std::vector<ElmType> get_list() {
        std::vector<ElmType> ret {};
        LenType elm_count = get_int<LenType>();

        for (LenType i = 0; i < elm_count; i++) {
            ret.push_back(*(ElmType*)get_byte_array(sizeof(ElmType)).data());
        }

        return ret;
    }

    std::string get_str() {
        std::vector<char> raw_str = get_list<uint16_t, char>();

        return std::string {raw_str.begin(), raw_str.end()};
    }

    bool get_next_tile(Tile& out);
    bool parse_tile_extra(Tile& tile);

    void parse_item_drops();

private:
    std::vector<uint8_t>::iterator m_curr_it {};
    std::vector<uint8_t> m_raw_data {};
};

}
