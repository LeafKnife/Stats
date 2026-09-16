#include "mod/Stats/StatsJsonCodec.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

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

void testSanitizesNegativeCounterValues() {
    stats::PlayerInfo info{"uuid-value", "xuid-value", "PlayerName"};
    stats::StatsData  data;
    auto              json = nlohmann::json::parse(stats::encodeStatsJson(info, data));
    json["minecraft:custom"]["minecraft:jump"] = -1;

    auto const decoded = stats::decodeStatsJson(json.dump());
    expectEqual(
        decoded.data.getMap(StatsType::custom)->at("minecraft:jump"),
        uint64_t{0},
        "sanitizes negative counter values"
    );
}

void testPageEncoding() {
    stats::query::StatsEntries entries{
        {"Alice", 9},
        {"Bob",   7},
        {"Carol", 5},
    };
    auto const page = stats::query::paginate(std::move(entries), 1, 2);
    auto const json = nlohmann::json::parse(stats::encodeStatsPageJson(page, "player"));
    expectEqual(json.at("page").get<int>(), 2, "encodes one-based page numbers");
    expectEqual(json.at("pageSize").get<int>(), 2, "encodes page size");
    expectEqual(json.at("total").get<int>(), 3, "encodes total entry count");
    expectEqual(json.at("totalPages").get<int>(), 2, "encodes total page count");
    expectEqual(json.at("items").size(), std::size_t{1}, "encodes only current page items");
    expectEqual(json.at("items")[0].at("player").get<std::string>(), std::string{"Carol"}, "encodes labels");
    expectEqual(json.at("items")[0].at("value").get<uint64_t>(), uint64_t{5}, "encodes values");
}

} // namespace

int runStatsJsonCodecTests() {
    testRoundTrip();
    testRequiredFields();
    testSanitizesNegativeCounterValues();
    testPageEncoding();
    return failures;
}
