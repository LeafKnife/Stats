#pragma once

#include "mc/world/actor/player/Player.h"
#include "mod/Stats/PlayerStats.h"
#include <memory>

namespace stats {
typedef std::map<mce::UUID, std::shared_ptr<PlayerStats>> PlayerStatsMap;

PlayerStatsMap&                           getPlayerStatsMap();
void load();
void unload();
} // namespace Stats