#include "mod/Stats/StatsQuery.h"
#include "mod/Stats/StatsRecord.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <Windows.h>
#include <Psapi.h>

namespace {

volatile uint64_t benchmarkChecksum = 0;

struct PlayerRecord {
    std::string         name;
    stats::StatsDataMap values;
};

std::vector<PlayerRecord> makeRecords(std::size_t count, std::size_t entriesPerPlayer) {
    std::vector<PlayerRecord> records;
    records.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        PlayerRecord record;
        record.name = "Player" + std::to_string(index);
        record.values.emplace("minecraft:stone", (index * 7919) % 100000);
        for (std::size_t entry = 1; entry < entriesPerPlayer; ++entry) {
            record.values.emplace("minecraft:key_" + std::to_string(entry), index + entry);
        }
        records.push_back(std::move(record));
    }
    return records;
}

stats::query::RankData buildWithViews(std::vector<PlayerRecord> const& records) {
    std::vector<stats::query::RankEntryView> entries;
    entries.reserve(records.size());
    for (auto const& record : records) entries.push_back({record.name, &record.values});
    return stats::query::buildRank(entries, "minecraft:stone");
}

stats::query::RankData buildDirect(std::vector<PlayerRecord> const& records, std::string const& key) {
    stats::query::RankBuilder builder(records.size(), key);
    for (auto const& record : records) builder.add(record.name, &record.values);
    return std::move(builder).finish();
}

stats::query::RankData buildFirstPage(std::vector<PlayerRecord> const& records) {
    stats::query::RankBuilder builder(records.size(), "minecraft:stone");
    for (auto const& record : records) builder.add(record.name, &record.values);
    return std::move(builder).finishPage(0).entries;
}

template <class Builder>
int64_t measure(std::vector<PlayerRecord> const& records, int iterations, Builder&& builder) {
    uint64_t    checksum = 0;
    auto const  start    = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < iterations; ++iteration) {
        auto rank = builder(records);
        checksum += rank.empty() ? 0 : rank.front().second;
    }
    auto const elapsed = std::chrono::steady_clock::now() - start;
    benchmarkChecksum = checksum;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / iterations;
}

int64_t median(std::vector<int64_t> samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void run(std::size_t players, int iterations) {
    auto const records = makeRecords(players, 25);
    buildWithViews(records);
    buildDirect(records, "minecraft:stone");

    std::vector<int64_t> viewSamples;
    std::vector<int64_t> directSamples;
    for (int sample = 0; sample < 9; ++sample) {
        if (sample % 2 == 0) {
            viewSamples.push_back(measure(records, iterations, buildWithViews));
            directSamples.push_back(
                measure(records, iterations, [](auto const& data) { return buildDirect(data, "minecraft:stone"); })
            );
        } else {
            directSamples.push_back(
                measure(records, iterations, [](auto const& data) { return buildDirect(data, "minecraft:stone"); })
            );
            viewSamples.push_back(measure(records, iterations, buildWithViews));
        }
    }
    auto const totals = measure(records, iterations, [](auto const& data) { return buildDirect(data, ""); });
    auto const page   = measure(records, iterations, buildFirstPage);
    std::cout << players << " players: views=" << median(std::move(viewSamples))
              << "us direct=" << median(std::move(directSamples)) << "us totals=" << totals
              << "us first-page=" << page << "us\n";
}

uint64_t privateBytes() {
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters));
    return counters.PrivateUsage;
}

struct FullPlayerRecord {
    stats::PlayerInfo info;
    stats::StatsData  data;
};

void runMemory(std::size_t players, std::size_t entriesPerType) {
    auto const before = privateBytes();
    std::vector<FullPlayerRecord> records;
    records.reserve(players);
    for (std::size_t player = 0; player < players; ++player) {
        FullPlayerRecord record;
        record.info = {
            "00000000-0000-0000-0000-" + std::to_string(100000000000ULL + player),
            std::to_string(1000000000000000ULL + player),
            "Player" + std::to_string(player),
        };
        for (auto const& descriptor : stats::StatsSchema) {
            stats::StatsDataMap values;
            values.reserve(entriesPerType);
            for (std::size_t entry = 0; entry < entriesPerType; ++entry) {
                auto const key = descriptor.type == StatsType::custom && entry < stats::CustomTypeMap.size()
                                   ? std::string{stats::CustomTypeMap[entry].second}
                                   : "minecraft:key_" + std::to_string(entry);
                values.emplace(key, player + entry);
            }
            record.data.loadMap(descriptor.type, std::move(values));
        }
        record.data.getMap(StatsType::custom);
        records.push_back(std::move(record));
    }
    auto const bytes = privateBytes() - before;
    std::cout << players << " players x " << entriesPerType << " keys/type: " << bytes / (1024.0 * 1024.0)
              << " MiB, " << bytes / static_cast<double>(players) << " bytes/player\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 4 && std::string_view{argv[1]} == "--memory") {
        runMemory(std::stoull(argv[2]), std::stoull(argv[3]));
        return 0;
    }
    run(1000, 200);
    run(10000, 30);
}
