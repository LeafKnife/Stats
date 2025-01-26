#pragma once
#include "lib/nlohmann/json.hpp"
#include "mod/Stats/CustomStatsType.h"

#include "mc/platform/UUID.h"
#include "mc/server/ServerPlayer.h"
namespace Stats {
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
    PlayerStats(ServerPlayer const& player);
    mce::UUID      getUuid();
    nlohmann::json getJson();
    bool           saveData();
    void           addStats(StatsDataType type, std::string key, int value = 1);
    void           addCustomStats(CustomType type, int value = 1);
    void           resetCustomStats(CustomType type, int value = 0);
};
typedef std::map<mce::UUID, PlayerStats*> PlayerStatsMap;
} // namespace Stats