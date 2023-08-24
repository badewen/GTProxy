#pragma once

#include <vector>
#include <string>

class World {
public:
    std::vector<uint8_t> data;
public:
    World(std::vector<uint8_t> d);
    World() = default;
};


