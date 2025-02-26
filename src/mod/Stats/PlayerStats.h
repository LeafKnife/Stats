#pragma once

#include "mod/Stats/StatsType.h"

#include <cstdint>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/dimension/Dimension.h>
#include <memory>
#include <nlohmann/json_fwd.hpp>


namespace stats {

class PlayerStats {
    struct MoveCache {
        uint64_t ride      = 0;
        uint64_t sneak     = 0;
        uint64_t climb     = 0;
        bool     isGliding = false;
    };

private:
    std::shared_ptr<StatsData> mData;
    mce::UUID                  mUuid;
    std::string                mXuid;
    std::string                mName;
    uint64_t                   mSneakingStartTick;
    std::filesystem::path      getPath();
    // void                  parseData(std::string const& data);

public:
    MoveCache mDistanceCache;
    Vec3      mLastPos;
    int       mLastDimensionId;
    // PlayerStats();
public:
    PlayerStats(Player const& player);
    mce::UUID      getUuid();
    nlohmann::json getJson();
    // nlohmann::json getJsonStatsData(StatsDataType type);
    bool saveData();
    void addStats(StatsDataType type, std::string key, uint64_t value = 1);
    void addCustomStats(CustomType type, uint64_t value = 1);
    void resetCustomStats(CustomType type, uint64_t value = 0);
    void startSneaking();
    void stopSneaking();
};
} // namespace stats