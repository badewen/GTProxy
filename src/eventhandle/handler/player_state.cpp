#include "player_state.h"

#include "../../eventmanager/event_manager.h"

static PlayerState s_playerState {};

static void onWorldEnter(const World& world) {
    s_playerState.CurrentWorld = world;
}

void PlayerStateHandler::Init() {
    event_manager::AddOnWorldEnter(onWorldEnter, "PlayerStateHandler");
}

const PlayerState& PlayerStateHandler::GetPlayerState() {
    return s_playerState;
}
