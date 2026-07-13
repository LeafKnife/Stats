#pragma once

#include "mod/Stats/StatsData.h"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stats::query {

using StatsEntries = std::vector<std::pair<std::string, uint64_t>>;
using RankData     = StatsEntries;

struct RankEntryView {
    std::string_view    playerName;
    StatsDataMap const* stats;
};

uint64_t getValue(StatsDataMap const& stats, std::string const& key);
StatsEntries buildDisplayEntries(StatsDataMap const& stats, StatsType type, uint64_t playTimeDelta);
RankData buildRank(std::span<RankEntryView const> entries, std::string const& key);

} // namespace stats::query
