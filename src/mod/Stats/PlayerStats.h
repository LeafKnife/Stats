#pragma once

#include "mod/Stats/StatsCustom.h"
#include "mod/Stats/StatsData.h"
#include "mod/Stats/StatsRecord.h"

#include <cstdint>
#include <mc/deps/core/math/Vec3.h>
#include <mc/platform/UUID.h>
#include <memory>
#include <string>


namespace stats {

struct PlayerSessionInit {
    mce::UUID   uuid;
    std::string xuid;
    std::string name;
    Vec3        position;
    int         dimensionId;
};

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
    PlayerStats(PlayerSessionInit init, std::shared_ptr<StatsData> data);
    mce::UUID           getUuid() const;
    PlayerInfo          getInfo() const;
    StatsData const&    getData() const;
    StatsDataMap const* getStatsMap(StatsType type) const;
    void                addStats(StatsType type, std::string const& key, uint64_t value = 1);
    void                addCustomStats(CustomType type, uint64_t value = 1);
    void                resetCustomStats(CustomType type, uint64_t value = 0);
    void                startSneaking(uint64_t currentTick);
    void                stopSneaking(uint64_t currentTick);
};
} // namespace stats
