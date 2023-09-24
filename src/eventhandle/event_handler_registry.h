
#pragma once

#include "handler/player_state.h"
#include "command/command_manager.h"

class EventHandlerRegistry {
public:
    static void Init() {
        PlayerStateHandler::Init();
        CommandManager::Init();
    }
};

