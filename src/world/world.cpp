#include "world.h"

#include <utility>

World::World(std::vector<uint8_t> d) { this->data = std::move(d); }
