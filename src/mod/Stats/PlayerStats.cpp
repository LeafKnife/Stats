#include "mod/Stats/PlayerStats.h"

#include <cstdint>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsFileRepository.h"

#include <memory>
#include <string>

using namespace ll::i18n_literals;

namespace stats {

PlayerStats::PlayerStats(Player const& player) {
    mUuid              = player.getUuid();
    mXuid              = player.getXuid();
    mName              = player.getRealName();
    mSneakingStartTick = 0;
    mLastPos           = const_cast<Vec3&>(player.getPosition());
    mLastDimensionId   = player.getDimensionId().id;

    auto const* cached           = findCachedStats(mUuid);
    bool const  needsInitialSave = !cached || !cached->second;
    if (!needsInitialSave) {
        mData = cached->second;
    } else {
        getLogger().debug("log.info.createData"_tr(mName));
        mData = std::make_shared<StatsData>();
    }

    PlayerInfo info = {mUuid.asString(), mXuid, mName};
    upsertStatsCache(std::make_pair(std::move(info), mData));
    if (needsInitialSave) saveData();
};
mce::UUID           PlayerStats::getUuid() const { return mUuid; };
StatsDataMap const* PlayerStats::getStatsMap(StatsType type) const {
    return static_cast<StatsData const&>(*mData).getMap(type);
}
bool PlayerStats::saveData() {
    getLogger().debug("log.info.savaData"_tr(mName));
    PlayerInfo const info{mUuid.asString(), mXuid, mName};
    return repository::save(info, *mData);
};
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
