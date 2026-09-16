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

inline constexpr std::size_t DefaultPageSize = 20;
inline constexpr std::size_t MaxPageSize     = 100;

struct StatsPage {
    StatsEntries entries;
    std::size_t  pageIndex;
    std::size_t  pageSize;
    std::size_t  totalEntries;
    std::size_t  totalPages;
};

struct RankEntryView {
    std::string_view    playerName;
    StatsDataMap const* stats;
};

class RankBuilder {
public:
    RankBuilder(std::size_t capacity, std::string key);

    void     add(std::string_view playerName, StatsDataMap const* stats);
    RankData finish() &&;
    StatsPage finishPage(std::size_t pageIndex, std::size_t pageSize = DefaultPageSize) &&;

private:
    RankData    mResult;
    std::string mKey;
};

uint64_t getValue(StatsDataMap const& stats, std::string const& key);
StatsEntries buildDisplayEntries(StatsDataMap const& stats, StatsType type, uint64_t playTimeDelta);
RankData buildRank(std::span<RankEntryView const> entries, std::string const& key);
void sortStatsEntries(StatsEntries& entries, bool alphabetical);
StatsPage paginate(StatsEntries entries, std::size_t pageIndex, std::size_t pageSize = DefaultPageSize);
StatsPage sortAndPaginate(
    StatsEntries entries,
    std::size_t  pageIndex,
    std::size_t  pageSize = DefaultPageSize,
    bool         alphabetical = false
);

} // namespace stats::query
