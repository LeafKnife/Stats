#include "mod/Stats/PlayerStats.h"

#include <cstdint>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>

#include <memory>
#include <string>
#include <utility>

namespace stats {

PlayerStats::PlayerStats(Player const& player, std::shared_ptr<StatsData> data) : mData(std::move(data)) {
    mUuid              = player.getUuid();
    mXuid              = player.getXuid();
    mName              = player.getRealName();
    mSneakingStartTick = 0;
    mLastPos           = player.getPosition();
    mLastDimensionId   = player.getDimensionId().id;
};
mce::UUID           PlayerStats::getUuid() const { return mUuid; };
PlayerInfo          PlayerStats::getInfo() const { return {mUuid.asString(), mXuid, mName}; }
StatsData const&    PlayerStats::getData() const { return *mData; }
StatsDataMap const* PlayerStats::getStatsMap(StatsType type) const {
    return static_cast<StatsData const&>(*mData).getMap(type);
}
void PlayerStats::addStats(StatsType type, std::string const& key, uint64_t value) {
    if (value <= 0) return;
    mData->add(type, key, value);
};
void PlayerStats::addCustomStats(CustomType type, uint64_t value) {
    if (value <= 0) return;
    mData->custom.add(type, value);
};

void PlayerStats::resetCustomStats(CustomType type, uint64_t value) { mData->custom.set(type, value); }

void PlayerStats::startSneaking() {
    mSneakingStartTick        = ll::service::getLevel()->getCurrentTick().tickID;
    mDistanceCache.isSneaking = true;
    mDistanceCache.sneak      = 0;
};
void PlayerStats::stopSneaking() {
    if (mSneakingStartTick == 0) return;
    auto record = ll::service::getLevel()->getCurrentTick().tickID - mSneakingStartTick;
    addCustomStats(CustomType::sneak_time, record);
    addCustomStats(CustomType::crouch_one_cm, mDistanceCache.sneak);
    mSneakingStartTick        = 0;
    mDistanceCache.isSneaking = false;
    mDistanceCache.sneak      = 0;
};
} // namespace stats
