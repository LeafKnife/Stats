#include "mod/Stats/StatsData.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string_view>
#include <unordered_set>

int runStatsDataTests() {
    static_assert(stats::StatsSchema.size() == static_cast<std::size_t>(StatsType::count) - 1);
    static_assert(stats::isValidStatsType(StatsType::custom));
    static_assert(stats::isValidStatsType(StatsType::killed_by));
    static_assert(!stats::isValidStatsType(static_cast<StatsType>(0)));
    static_assert(!stats::isValidStatsType(StatsType::count));
    static_assert(stats::isStatsSchemaValid());
    static_assert(!stats::isBlockOrItemStatsCategory(StatsType::mined));
    static_assert(!stats::isBlockOrItemStatsCategory(StatsType::broken));
    static_assert(stats::isBlockOrItemStatsCategory(StatsType::used));
    static_assert(!stats::isBlockOrItemStatsCategory(StatsType::killed));
    static_assert(stats::hasTranslation("石头", "block.minecraft:stone"));
    static_assert(!stats::hasTranslation("", "block.minecraft:stone"));
    static_assert(!stats::hasTranslation("item.minecraft:stone", "item.minecraft:stone"));

    int failures = 0;

    std::unordered_set<std::string_view> schemaKeys;
    schemaKeys.reserve(stats::StatsSchema.size());
    for (std::size_t index = 0; index < stats::StatsSchema.size(); ++index) {
        auto const& descriptor = stats::StatsSchema[index];
        if (static_cast<std::size_t>(descriptor.type) != index + 1) {
            std::cerr << "FAILED: StatsType index mismatch at " << index << '\n';
            ++failures;
        }
        if (descriptor.key.empty()) {
            std::cerr << "FAILED: empty StatsType key at " << index << '\n';
            ++failures;
        }
        if (!schemaKeys.emplace(descriptor.key).second) {
            std::cerr << "FAILED: duplicate StatsType key " << descriptor.key << '\n';
            ++failures;
        }
        if (stats::getStatsTypeIndex(descriptor.type) != index
            || stats::getStatsTypeKey(descriptor.type) != descriptor.key) {
            std::cerr << "FAILED: StatsType lookup mismatch at " << index << '\n';
            ++failures;
        }
    }

    if (stats::getStatsEntryTranslationKey(StatsType::mined, "minecraft:stone")
        != "block.minecraft:stone") {
        std::cerr << "FAILED: mined stats use block translation keys\n";
        ++failures;
    }
    if (!stats::getStatsEntryTranslationKey(StatsType::used, "minecraft:arrow").empty()) {
        std::cerr << "FAILED: mixed block/item stats must not select one source key\n";
        ++failures;
    }
    if (stats::getStatsEntryTranslationKey(StatsType::killed_by, "minecraft:arrow")
        != "entity.minecraft:arrow") {
        std::cerr << "FAILED: combat stats use entity translation keys\n";
        ++failures;
    }
    if (stats::getStatsCategoryTranslationKey(StatsType::custom) != "stats.category.custom") {
        std::cerr << "FAILED: stats categories use category translation keys\n";
        ++failures;
    }
    if (stats::getStatsEntryTranslationKey(StatsType::custom, "minecraft:jump")
        != "stats.custom.minecraft:jump") {
        std::cerr << "FAILED: custom stats use their own translation keys\n";
        ++failures;
    }

    stats::StatsDataMap source{
        {"minecraft:jump",           9  },
        {"minecraft:walk_one_cm",    120},
        {"third_party:custom_value", 17 },
        {"minecraft:deaths",         0  },
    };

    stats::StatsData data;
    data.custom.load(source);
    if (data.custom.asMap() != source) {
        std::cerr << "FAILED: custom stats preserve loaded JSON values\n";
        ++failures;
    }

    data.custom.add(stats::CustomType::jump, 3);
    if (data.custom.asMap().at("minecraft:jump") != 12) {
        std::cerr << "FAILED: known custom stat uses array-backed update\n";
        ++failures;
    }

    data.add(StatsType::custom, "third_party:custom_value", 5);
    if (data.custom.asMap().at("third_party:custom_value") != 22) {
        std::cerr << "FAILED: unknown custom stat survives generic update\n";
        ++failures;
    }

    data.custom.set(stats::CustomType::deaths, 4);
    auto const* customMap = data.getMap(StatsType::custom);
    if (!customMap || customMap->at("minecraft:deaths") != 4) {
        std::cerr << "FAILED: custom stat snapshot reflects reset values\n";
        ++failures;
    }

    data.add(StatsType::mined, "minecraft:stone", 2);
    auto const* minedMap = data.getMap(StatsType::mined);
    if (!minedMap || minedMap->at("minecraft:stone") != 2) {
        std::cerr << "FAILED: non-custom stats still use map storage\n";
        ++failures;
    }

    data.custom.set(stats::CustomType::jump, std::numeric_limits<uint64_t>::max() - 1);
    data.custom.add(stats::CustomType::jump, 2);
    if (data.custom.asMap().at("minecraft:jump") != std::numeric_limits<uint64_t>::max()) {
        std::cerr << "FAILED: custom stats saturate instead of wrapping\n";
        ++failures;
    }

    data.add(StatsType::mined, "minecraft:stone", std::numeric_limits<uint64_t>::max());
    if (data.getMap(StatsType::mined)->at("minecraft:stone") != std::numeric_limits<uint64_t>::max()) {
        std::cerr << "FAILED: category stats saturate instead of wrapping\n";
        ++failures;
    }

    return failures;
}
