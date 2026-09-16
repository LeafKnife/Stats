#pragma once

#include "mod/Stats/StatsData.h"
#include <cstddef>
#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h>
#include <optional>
#include <string>

namespace stats::form {
using StatsPair = std::pair<std::string, uint64_t>;
void sendMainGui(Player& player);
void sendStatsGui(Player& player, StatsType type, std::size_t pageIndex = 0);
void sendStatsGui(
    Player&     player,
    mce::UUID   targetUuid,
    std::string targetName,
    StatsType   type,
    std::size_t pageIndex = 0
);
void sendRankGui(Player& player, StatsType statsType, std::string type, std::size_t pageIndex = 0);

std::optional<std::string> renderStatsContent(mce::UUID uuid, StatsType type);


} // namespace stats::form
