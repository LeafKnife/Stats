#pragma once
#include "mc/world/actor/Actor.h"
#include "mod/Stats/PlayerStats.h"

namespace Stats::event {
void            listenEvents();
void            removeEvents();
::Block const& getBlockByBlockPos(const BlockPos& pos, int dimensionId);
PlayerStatsMap& getPlayerStatsMap();
} // namespace Stats::event

namespace Stats::event::hook {
void hook();
void unhook();
} // namespace Stats::event::hook