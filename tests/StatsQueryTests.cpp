#include "mod/Stats/StatsQuery.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

int runStatsCustomTests();
int runStatsDataTests();

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
    testValueLookup();
    testTotalsUse64Bits();
    testRanking();
    testEmptyRanking();

    if (failures != 0) return 1;
    std::cout << "Stats tests passed\n";
    return 0;
}
