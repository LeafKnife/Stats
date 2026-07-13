#include "mod/Stats/PlayerStats.h"

#include <cstdint>
#include <exception>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Stats.h"

#include <memory>
#include <nlohmann/json.hpp>
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

    if (auto const* cached = findCachedStats(mUuid)) {
        mData = cached->second;
    } else {
        getLogger().debug("log.info.createData"_tr(mName));
        mData           = std::make_shared<StatsData>();
        PlayerInfo info = {mUuid.asString(), mXuid, mName};
        addStatsCache(std::make_pair(std::move(info), mData));
        saveData();
    }
};
mce::UUID             PlayerStats::getUuid() const { return mUuid; };
std::filesystem::path PlayerStats::getPath() const { return getStatsPath().concat("/" + mUuid.asString() + ".json"); };
nlohmann::json        PlayerStats::getJson() const {
    nlohmann::json j = {
        {"playerInfo",          {{"uuid", mUuid.asString()}, {"xuid", mXuid}, {"name", mName}}},
        {"minecraft:custom",    *mData->getMap(StatsType::custom)                             },
        {"minecraft:mined",     mData->mined                                                  },
        {"minecraft:broken",    mData->broken                                                 },
        {"minecraft:crafted",   mData->crafted                                                },
        {"minecraft:used",      mData->used                                                   },
        {"minecraft:picked_up", mData->picked_up                                              },
        {"minecraft:dropped",   mData->dropped                                                },
        {"minecraft:killed",    mData->killed                                                 },
        {"minecraft:killed_by", mData->killed_by                                              },
    };
    return j;
}
StatsDataMap const* PlayerStats::getStatsMap(StatsType type) const {
    return static_cast<StatsData const&>(*mData).getMap(type);
}
bool PlayerStats::saveData() {
    getLogger().debug("log.info.savaData"_tr(mName));
    auto j = getJson();
    try {
        return ll::file_utils::writeFile(getPath(), j.dump());
    } catch (std::exception& exception) {
        getLogger().error(exception.what());
        return false;
    }
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
