#include "mod/Stats/StatsQuery.h"

#include <algorithm>
#include <numeric>

namespace stats::query {
namespace {

bool valueOrder(StatsEntries::value_type const& left, StatsEntries::value_type const& right) {
    return left.second != right.second ? left.second > right.second : left.first < right.first;
}

bool nameOrder(StatsEntries::value_type const& left, StatsEntries::value_type const& right) {
    return left.first < right.first;
}

} // namespace

RankBuilder::RankBuilder(std::size_t capacity, std::string key) : mKey(std::move(key)) {
    mResult.reserve(capacity);
}

void RankBuilder::add(std::string_view playerName, StatsDataMap const* stats) {
    if (!stats) return;
    mResult.emplace_back(playerName, getValue(*stats, mKey));
}

RankData RankBuilder::finish() && {
    std::sort(mResult.begin(), mResult.end(), valueOrder);
    return std::move(mResult);
}

StatsPage RankBuilder::finishPage(std::size_t pageIndex, std::size_t pageSize) && {
    return sortAndPaginate(std::move(mResult), pageIndex, pageSize);
}

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
    RankBuilder builder(entries.size(), key);
    for (auto const& entry : entries) {
        builder.add(entry.playerName, entry.stats);
    }
    return std::move(builder).finish();
}

void sortStatsEntries(StatsEntries& entries, bool alphabetical) {
    if (alphabetical) {
        std::sort(entries.begin(), entries.end(), nameOrder);
        return;
    }
    std::sort(entries.begin(), entries.end(), valueOrder);
}

StatsPage paginate(StatsEntries entries, std::size_t pageIndex, std::size_t pageSize) {
    pageSize             = std::clamp(pageSize, std::size_t{1}, MaxPageSize);
    auto const total     = entries.size();
    auto const pageCount = total == 0 ? std::size_t{0} : (total + pageSize - 1) / pageSize;
    if (pageCount == 0) {
        return {{}, 0, pageSize, 0, 0};
    }

    pageIndex       = std::min(pageIndex, pageCount - 1);
    auto const begin = pageIndex * pageSize;
    auto const end   = std::min(begin + pageSize, total);

    StatsEntries pageEntries;
    pageEntries.reserve(end - begin);
    for (auto index = begin; index < end; ++index) {
        pageEntries.push_back(std::move(entries[index]));
    }
    return {std::move(pageEntries), pageIndex, pageSize, total, pageCount};
}

StatsPage sortAndPaginate(StatsEntries entries, std::size_t pageIndex, std::size_t pageSize, bool alphabetical) {
    pageSize             = std::clamp(pageSize, std::size_t{1}, MaxPageSize);
    auto const total     = entries.size();
    auto const pageCount = total == 0 ? std::size_t{0} : (total + pageSize - 1) / pageSize;
    if (pageCount == 0) return {{}, 0, pageSize, 0, 0};

    pageIndex       = std::min(pageIndex, pageCount - 1);
    auto const end = std::min((pageIndex + 1) * pageSize, total);
    std::partial_sort(
        entries.begin(),
        entries.begin() + static_cast<std::ptrdiff_t>(end),
        entries.end(),
        alphabetical ? nameOrder : valueOrder
    );
    return paginate(std::move(entries), pageIndex, pageSize);
}

} // namespace stats::query
