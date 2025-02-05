#pragma once

#include "mod/Stats/PlayerStats.h"

namespace stats {
typedef std::map<mce::UUID, PlayerStats*> PlayerStatsMap;

PlayerStatsMap&                           getPlayerStatsMap();
void load();
void unload();
} // namespace Stats