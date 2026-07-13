#pragma once

#include "mod/Stats/StatsCustom.h"

#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
enum class StatsType : int { custom = 1, mined, broken, crafted, used, picked_up, dropped, killed, killed_by };
namespace stats {
using StatsDataMap = std::unordered_map<std::string, uint64_t>;

inline const std::map<StatsType, std::string> StatsTypeMap = {
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

class CustomStatsData {
public:
    void                load(StatsDataMap const& values);
    void                add(CustomType type, uint64_t value = 1);
    void                add(std::string const& key, uint64_t value = 1);
    void                set(CustomType type, uint64_t value = 0);
    StatsDataMap const& asMap() const;

private:
    std::array<uint64_t, static_cast<std::size_t>(CustomType::count)> mValues{};
    std::bitset<static_cast<std::size_t>(CustomType::count)>          mPresent;
    StatsDataMap                                                      mUnknown;
    mutable StatsDataMap                                              mSnapshot;
    mutable bool                                                      mSnapshotDirty = true;
};

struct StatsData {
    CustomStatsData     custom;
    StatsDataMap        mined;
    StatsDataMap        broken;
    StatsDataMap        crafted;
    StatsDataMap        used;
    StatsDataMap        picked_up;
    StatsDataMap        dropped;
    StatsDataMap        killed;
    StatsDataMap        killed_by;
    void                add(StatsType type, std::string const& key, uint64_t value = 1);
    StatsDataMap const* getMap(StatsType type) const;
};
} // namespace stats
