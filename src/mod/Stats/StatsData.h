#pragma once

#include "mod/Stats/StatsCustom.h"

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
enum class StatsType : int { custom = 1, mined, broken, crafted, used, picked_up, dropped, killed, killed_by, count };
namespace stats {
using StatsDataMap = std::unordered_map<std::string, uint64_t>;

struct StatsDescriptor {
    StatsType        type;
    std::string_view key;
};

inline constexpr std::array<StatsDescriptor, static_cast<std::size_t>(StatsType::count) - 1> StatsSchema{
    {
     {StatsType::custom, "minecraft:custom"},
     {StatsType::mined, "minecraft:mined"},
     {StatsType::broken, "minecraft:broken"},
     {StatsType::crafted, "minecraft:crafted"},
     {StatsType::used, "minecraft:used"},
     {StatsType::picked_up, "minecraft:picked_up"},
     {StatsType::dropped, "minecraft:dropped"},
     {StatsType::killed, "minecraft:killed"},
     {StatsType::killed_by, "minecraft:killed_by"},
     }
};

constexpr bool isValidStatsType(StatsType type) {
    auto const value = static_cast<int>(type);
    return value >= static_cast<int>(StatsType::custom) && value < static_cast<int>(StatsType::count);
}

constexpr std::size_t getStatsTypeIndex(StatsType type) {
    return static_cast<std::size_t>(type) - static_cast<std::size_t>(StatsType::custom);
}

constexpr std::string_view getStatsTypeKey(StatsType type) {
    return isValidStatsType(type) ? StatsSchema[getStatsTypeIndex(type)].key : std::string_view{};
}

constexpr bool isStatsSchemaValid() {
    for (std::size_t index = 0; index < StatsSchema.size(); ++index) {
        auto const& descriptor = StatsSchema[index];
        if (getStatsTypeIndex(descriptor.type) != index || descriptor.key.empty()) return false;
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (StatsSchema[previous].key == descriptor.key) return false;
        }
    }
    return true;
}

static_assert(isStatsSchemaValid());

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
