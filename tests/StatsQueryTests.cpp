#include "mod/Stats/StatsQuery.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

int runStatsCustomTests();
int runStatsDataTests();
int runStatsJsonCodecTests();

namespace {

int failures = 0;

template <class Actual, class Expected>
void expectEqual(Actual const& actual, Expected const& expected, char const* testName) {
    if (actual == expected) return;
    std::cerr << "FAILED: " << testName << '\n';
    ++failures;
}

void testValueLookup() {
    stats::StatsDataMap values{
        {"stone", 7},
        {"dirt",  3}
    };
    expectEqual(stats::query::getValue(values, "stone"), uint64_t{7}, "finds an existing key");
    expectEqual(stats::query::getValue(values, "missing"), uint64_t{0}, "returns zero for a missing key");
}

void testTotalsUse64Bits() {
    stats::StatsDataMap values{
        {"large", static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())},
        {"extra", 42                                                         },
    };
    expectEqual(
        stats::query::getValue(values, ""),
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 42,
        "sums values without 32-bit overflow"
    );
}

uint64_t findEntryValue(stats::query::StatsEntries const& entries, std::string const& key) {
    for (auto const& entry : entries) {
        if (entry.first == key) return entry.second;
    }
    return 0;
}

void testDisplayEntriesAddActivePlayTime() {
    stats::StatsDataMap values{
        {"minecraft:play_time", 100},
        {"minecraft:jump",      7  },
    };

    auto const entries = stats::query::buildDisplayEntries(values, StatsType::custom, 25);
    expectEqual(entries.size(), std::size_t{2}, "keeps existing custom entries");
    expectEqual(findEntryValue(entries, "minecraft:play_time"), uint64_t{125}, "adds active play time");
    expectEqual(values.at("minecraft:play_time"), uint64_t{100}, "does not mutate stored play time");
}

void testDisplayEntriesInsertMissingPlayTime() {
    stats::StatsDataMap values{
        {"minecraft:jump", 7}
    };

    auto const entries = stats::query::buildDisplayEntries(values, StatsType::custom, 25);
    expectEqual(entries.size(), std::size_t{2}, "adds a missing play-time entry");
    expectEqual(findEntryValue(entries, "minecraft:play_time"), uint64_t{25}, "uses active play time for new entry");
}

void testRanking() {
    stats::StatsDataMap alice{
        {"stone", 8},
        {"dirt",  2}
    };
    stats::StatsDataMap bob{
        {"stone", 15}
    };
    stats::StatsDataMap carol{
        {"dirt", 5}
    };
    std::vector<stats::query::RankEntryView> entries{
        {"Alice", &alice},
        {"Bob",   &bob  },
        {"Carol", &carol},
    };

    auto const rank = stats::query::buildRank(entries, "stone");
    expectEqual(rank.size(), std::size_t{3}, "keeps every player in the ranking");
    expectEqual(rank[0], stats::query::RankData::value_type{"Bob", 15}, "sorts highest value first");
    expectEqual(rank[1], stats::query::RankData::value_type{"Alice", 8}, "sorts the second value");
    expectEqual(rank[2], stats::query::RankData::value_type{"Carol", 0}, "includes missing values as zero");
}

void testEmptyRanking() {
    std::vector<stats::query::RankEntryView> entries;
    expectEqual(stats::query::buildRank(entries, "").size(), std::size_t{0}, "handles an empty ranking");
}

} // namespace

int main() {
    failures += runStatsCustomTests();
    failures += runStatsDataTests();
    failures += runStatsJsonCodecTests();
    testValueLookup();
    testTotalsUse64Bits();
    testDisplayEntriesAddActivePlayTime();
    testDisplayEntriesInsertMissingPlayTime();
    testRanking();
    testEmptyRanking();

    if (failures != 0) return 1;
    std::cout << "Stats tests passed\n";
    return 0;
}
