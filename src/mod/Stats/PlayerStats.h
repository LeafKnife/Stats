#pragma once

#include "mod/Stats/CustomStatsType.h"

#include "nlohmann/json_fwd.hpp"
#include "mc/server/ServerPlayer.h"
namespace stats {
enum class StatsDataType { custom, mined, broken, crafted, used, picked_up, dropped, killed, killed_by };

typedef std::map<std::string, int> StatsDataMap;
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
};

class PlayerStats {
private:
    StatsData             mData;
    mce::UUID             mUuid;
    std::string           mXuid;
    std::string           mName;
    std::filesystem::path getPath();
    void                  parseData(std::string const& data);

public:
    // PlayerStats();
    PlayerStats(Player const& player);
    mce::UUID      getUuid();
    nlohmann::json getJson();
    bool           saveData();
    void           addStats(StatsDataType type, std::string key, int value = 1);
    void           addCustomStats(CustomType type, int value = 1);
    void           resetCustomStats(CustomType type, int value = 0);
};
} // namespace Stats