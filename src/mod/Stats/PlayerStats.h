#pragma once

#include "mc/server/ServerPlayer.h"
#include "mod/Stats/StatsType.h"
#include <nlohmann/json_fwd.hpp>
#include <memory>


namespace stats {

class PlayerStats {
private:
    std::shared_ptr<StatsData> mData;
    mce::UUID                  mUuid;
    std::string                mXuid;
    std::string                mName;
    std::filesystem::path      getPath();
    // void                  parseData(std::string const& data);

public:
    // PlayerStats();
    PlayerStats(Player const& player);
    mce::UUID      getUuid();
    nlohmann::json getJson();
    // nlohmann::json getJsonStatsData(StatsDataType type);
    bool saveData();
    void addStats(StatsDataType type, std::string key, int value = 1);
    void addCustomStats(CustomType type, int value = 1);
    void resetCustomStats(CustomType type, int value = 0);
};
} // namespace stats