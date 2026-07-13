#include "mod/Stats/StatsQuery.h"

#include <algorithm>
#include <numeric>

namespace stats::query {

uint64_t getValue(StatsDataMap const& stats, std::string const& key) {
    if (key.empty()) {
        return std::accumulate(stats.begin(), stats.end(), uint64_t{0}, [](uint64_t total, auto const& pair) {
            return total + pair.second;
        });
    }

    auto const value = stats.find(key);
    return value == stats.end() ? 0 : value->second;
}

StatsEntries buildDisplayEntries(StatsDataMap const& stats, StatsType type, uint64_t playTimeDelta) {
    constexpr std::string_view playTimeKey = "minecraft:play_time";

    StatsEntries result;
    result.reserve(stats.size() + (type == StatsType::custom ? 2 : 0));

    bool hasPlayTime = false;
    for (auto const& [key, value] : stats) {
        if (type == StatsType::custom && key == playTimeKey) {
            result.emplace_back(key, value + playTimeDelta);
            hasPlayTime = true;
        } else {
            result.emplace_back(key, value);
        }
    }
    if (type == StatsType::custom && !hasPlayTime) {
        result.emplace_back(playTimeKey, playTimeDelta);
    }
    return result;
}

RankData buildRank(std::span<RankEntryView const> entries, std::string const& key) {
    RankData result;
    result.reserve(entries.size());

    for (auto const& entry : entries) {
        if (!entry.stats) continue;
        result.emplace_back(entry.playerName, getValue(*entry.stats, key));
    }

    std::sort(result.begin(), result.end(), [](auto const& left, auto const& right) {
        return left.second > right.second;
    });
    return result;
}

} // namespace stats::query
