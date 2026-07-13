#include "mod/Stats/StatsData.h"

#include <cstdint>
#include <iostream>

int runStatsDataTests() {
    int failures = 0;

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

    return failures;
}
