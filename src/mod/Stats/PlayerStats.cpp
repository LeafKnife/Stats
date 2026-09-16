#include "mod/Stats/PlayerStats.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace stats {

PlayerStats::PlayerStats(PlayerSessionInit init, std::shared_ptr<StatsData> data)
: mData(std::move(data)),
  mUuid(init.uuid),
  mXuid(std::move(init.xuid)),
  mName(std::move(init.name)),
  mSneakingStartTick(0),
  mLastCheckpointTick(init.currentTick),
  mLastPos(init.position),
  mLastDimensionId(init.dimensionId) {}
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

void PlayerStats::startSneaking(uint64_t currentTick) {
    mSneakingStartTick        = currentTick;
    mDistanceCache.isSneaking = true;
    mDistanceCache.sneak      = 0;
};
void PlayerStats::stopSneaking(uint64_t currentTick) {
    if (mSneakingStartTick == 0) return;
    if (currentTick < mSneakingStartTick) {
        mSneakingStartTick        = 0;
        mDistanceCache.isSneaking = false;
        mDistanceCache.sneak      = 0;
        return;
    }
    auto record = currentTick - mSneakingStartTick;
    addCustomStats(CustomType::sneak_time, record);
    addCustomStats(CustomType::crouch_one_cm, mDistanceCache.sneak);
    mSneakingStartTick        = 0;
    mDistanceCache.isSneaking = false;
    mDistanceCache.sneak      = 0;
};

void PlayerStats::checkpoint(uint64_t currentTick) {
    if (currentTick < mLastCheckpointTick) {
        mLastCheckpointTick = currentTick;
        return;
    }

    addCustomStats(CustomType::play_time, currentTick - mLastCheckpointTick);
    mLastCheckpointTick = currentTick;

    if (!mDistanceCache.isSneaking || mSneakingStartTick == 0) return;
    addCustomStats(CustomType::sneak_time, currentTick - mSneakingStartTick);
    addCustomStats(CustomType::crouch_one_cm, mDistanceCache.sneak);
    mSneakingStartTick   = currentTick;
    mDistanceCache.sneak = 0;
}

uint64_t PlayerStats::getPendingPlayTime(uint64_t currentTick) const {
    return currentTick < mLastCheckpointTick ? 0 : currentTick - mLastCheckpointTick;
}
} // namespace stats
