#pragma once

#include "mod/Stats/StatsCustom.h"
#include "mod/Stats/StatsData.h"
#include "mod/Stats/StatsRecord.h"

#include <cstdint>
#include <memory>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/dimension/Dimension.h>
#include <string>


namespace stats {

class PlayerStats {
    struct MoveCache {
        uint64_t ride        = 0;
        uint64_t sneak       = 0;
        bool     isGliding   = false;
        bool     isSneaking  = false;
        bool     isSprinting = false;
    };

private:
    std::shared_ptr<StatsData> mData;
    mce::UUID                  mUuid;
    std::string                mXuid;
    std::string                mName;
    uint64_t                   mSneakingStartTick;

public:
    MoveCache mDistanceCache;
    Vec3      mLastPos;
    int       mLastDimensionId;
    // PlayerStats();
public:
    PlayerStats(Player const& player, std::shared_ptr<StatsData> data);
    mce::UUID           getUuid() const;
    PlayerInfo          getInfo() const;
    StatsData const&    getData() const;
    StatsDataMap const* getStatsMap(StatsType type) const;
    void                addStats(StatsType type, std::string const& key, uint64_t value = 1);
    void                addCustomStats(CustomType type, uint64_t value = 1);
    void                resetCustomStats(CustomType type, uint64_t value = 0);
    void                startSneaking();
    void                stopSneaking();
};
} // namespace stats
