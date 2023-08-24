
#pragma once

#include "handler/player_state.h"

class EventHandlerRegistry {
public:
    static void Init() {
        PlayerStateHandler::Init();
    }
};

