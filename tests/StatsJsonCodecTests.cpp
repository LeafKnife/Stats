#include "mod/Stats/StatsJsonCodec.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

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
    for (std::size_t index = 0; index < stats::StatsSchema.size(); ++index) {
        auto const& descriptor = stats::StatsSchema[index];
        auto        values     = stats::StatsDataMap{
                       {std::string(descriptor.key) + ":test", index + 1}
        };
        if (descriptor.type == StatsType::custom) {
            values["minecraft:jump"]           = 7;
            values["third_party:custom_value"] = 9;
        } else if (descriptor.type == StatsType::mined) {
            values["minecraft:stone"] = std::numeric_limits<uint64_t>::max();
        }
        data.loadMap(descriptor.type, std::move(values));
    }

    auto const decoded = stats::decodeStatsJson(stats::encodeStatsJson(info, data));
    expectEqual(decoded.info.uuid, info.uuid, "preserves player UUID");
    expectEqual(decoded.info.xuid, info.xuid, "preserves player XUID");
    expectEqual(decoded.info.name, info.name, "preserves player name");
    for (auto const& descriptor : stats::StatsSchema) {
        auto const* expected = data.getMap(descriptor.type);
        auto const* actual   = decoded.data.getMap(descriptor.type);
        if (!expected || !actual) {
            std::cerr << "FAILED: schema type has no data map " << descriptor.key << '\n';
            ++failures;
            continue;
        }
        expectEqual(*actual, *expected, "preserves schema-driven stats");
    }
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
