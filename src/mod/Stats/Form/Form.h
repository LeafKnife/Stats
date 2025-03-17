#pragma once

#include "mod/Stats/StatsData.h"
#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h>

namespace stats::form {
using StatsPair = std::pair<std::string, uint64_t>;
void sendMainGui(Player& player);
void sendStatsGui(Player& player, StatsType type);
void sendRankGui(Player& player, StatsType statsType, std::string type);

std::optional<std::string> renderStatsContent(mce::UUID uuid, StatsType type, uint64_t tick = 0);


} // namespace stats::form