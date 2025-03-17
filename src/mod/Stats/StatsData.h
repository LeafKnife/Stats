#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
enum class StatsType : int { custom = 1, mined, broken, crafted, used, picked_up, dropped, killed, killed_by };
namespace stats {
using StatsDataMap = std::unordered_map<std::string, uint64_t>;

static const std::map<StatsType, std::string> StatsTypeMap = {
    {StatsType::custom,    "minecraft:custom"   },
    {StatsType::mined,     "minecraft:mined"    },
    {StatsType::broken,    "minecraft:broken"   },
    {StatsType::crafted,   "minecraft:crafted"  },
    {StatsType::used,      "minecraft:used"     },
    {StatsType::picked_up, "minecraft:picked_up"},
    {StatsType::dropped,   "minecraft:dropped"  },
    {StatsType::killed,    "minecraft:killed"   },
    {StatsType::killed_by, "minecraft:killed_by"},
};

struct StatsData {
    StatsDataMap custom;
    StatsDataMap mined;
    StatsDataMap broken;
    StatsDataMap crafted;
    StatsDataMap used;
    StatsDataMap picked_up;
    StatsDataMap dropped;
    StatsDataMap killed;
    StatsDataMap killed_by;
    StatsDataMap* getMap(StatsType type);
};
} // namespace stats