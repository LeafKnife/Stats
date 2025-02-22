#pragma once

#include "mc/server/ServerPlayer.h"
#include "mod/Stats/StatsType.h"
#include <memory>
#include <nlohmann/json_fwd.hpp>


namespace stats {

class PlayerStats {
private:
    std::shared_ptr<StatsData> mData;
    mce::UUID                  mUuid;
    std::string                mXuid;
    std::string                mName;
    uint64_t                   mSneakingStartTick;
    std::filesystem::path      getPath();
    // void                  parseData(std::string const& data);

public:
    // PlayerStats();
    PlayerStats(Player const& player);
    mce::UUID      getUuid();
    nlohmann::json getJson();
    // nlohmann::json getJsonStatsData(StatsDataType type);
    bool saveData();
    void addStats(StatsDataType type, std::string key, uint64_t value = 1);
    void addCustomStats(CustomType type, uint64_t value = 1);
    void resetCustomStats(CustomType type, uint64_t value = 0);
    void startSneaking();
    void addSneakTick();
};
} // namespace stats