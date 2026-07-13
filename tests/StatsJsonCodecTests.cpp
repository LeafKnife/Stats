#include "mod/Stats/StatsJsonCodec.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

namespace {

int failures = 0;

template <class Actual, class Expected>
void expectEqual(Actual const& actual, Expected const& expected, char const* testName) {
    if (actual == expected) return;
    std::cerr << "FAILED: " << testName << '\n';
    ++failures;
}

void testRoundTrip() {
    stats::PlayerInfo info{"uuid-value", "xuid-value", "PlayerName"};
    stats::StatsData  data;
    data.custom.load({
        {"minecraft:jump",           7},
        {"third_party:custom_value", 9},
    });
    data.mined["minecraft:stone"]         = std::numeric_limits<uint64_t>::max();
    data.broken["minecraft:pickaxe"]      = 2;
    data.crafted["minecraft:crafting"]    = 3;
    data.used["minecraft:used"]           = 4;
    data.picked_up["minecraft:picked_up"] = 5;
    data.dropped["minecraft:dropped"]     = 6;
    data.killed["minecraft:killed"]       = 7;
    data.killed_by["minecraft:killed_by"] = 8;

    auto const decoded = stats::decodeStatsJson(stats::encodeStatsJson(info, data));
    expectEqual(decoded.info.uuid, info.uuid, "preserves player UUID");
    expectEqual(decoded.info.xuid, info.xuid, "preserves player XUID");
    expectEqual(decoded.info.name, info.name, "preserves player name");
    expectEqual(decoded.data.custom.asMap(), data.custom.asMap(), "preserves custom stats");
    expectEqual(decoded.data.mined, data.mined, "preserves 64-bit mined stats");
    expectEqual(decoded.data.broken, data.broken, "preserves broken stats");
    expectEqual(decoded.data.crafted, data.crafted, "preserves crafted stats");
    expectEqual(decoded.data.used, data.used, "preserves used stats");
    expectEqual(decoded.data.picked_up, data.picked_up, "preserves picked-up stats");
    expectEqual(decoded.data.dropped, data.dropped, "preserves dropped stats");
    expectEqual(decoded.data.killed, data.killed, "preserves killed stats");
    expectEqual(decoded.data.killed_by, data.killed_by, "preserves killed-by stats");
}

void testRequiredFields() {
    bool threw = false;
    try {
        static_cast<void>(stats::decodeStatsJson("{}"));
    } catch (std::exception const&) {
        threw = true;
    }
    expectEqual(threw, true, "rejects missing required fields");
}

} // namespace

int runStatsJsonCodecTests() {
    testRoundTrip();
    testRequiredFields();
    return failures;
}
