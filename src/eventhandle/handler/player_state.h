#pragma once

#include "../../world/world.h"

struct PlayerState {
    World CurrentWorld;
};

class PlayerStateHandler {
public:
    static void Init();
    static const PlayerState& GetPlayerState();
};

