#pragma once

#include <mc/world/actor/player/Player.h>
#include "mod/Stats/StatsType.h"

namespace stats::form {
void sendMainGui(Player& player);
void sendStatsGui(Player& player, StatsDataType type);
void sendRankGui(Player& player, StatsDataType statsType,std::string type);
// void sendEntityStatsGui(Player& player,bool type = false);
// void sendItemStatsGui(Player& player,StatsDataType type);
// void sendCustomStatsGui(Player& player);
} // namespace stats::form