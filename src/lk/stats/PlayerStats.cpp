#include "lk/stats/PlayerStats.h"

#include <cstdint>
#include <exception>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>

#include "lk/stats/Stats.h"

#include <algorithm>
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

    auto                 cache = getStatsCache();
    auto                 uuid  = mUuid.asString();
    StatsCache::iterator it    = std::find_if(cache.begin(), cache.end(), [&uuid](const StatsCacheData& data) {
        return data.first.uuid.compare(uuid) == 0;
    });
    if (it != cache.end()) {
        mData = it->second;
    } else {
        getLogger().debug("log.info.createData"_tr(mName));
        mData           = std::make_shared<StatsData>();
        PlayerInfo info = {mUuid.asString(), mXuid, mName};
        cache.push_back(std::make_pair(info, mData));
        saveData();
    }
};
mce::UUID             PlayerStats::getUuid() { return mUuid; };
std::filesystem::path PlayerStats::getPath() { return getStatsPath().concat("/" + mUuid.asString() + ".json"); };
nlohmann::json        PlayerStats::getJson() {
    nlohmann::json j = {
        {"playerInfo",          {{"uuid", mUuid.asString()}, {"xuid", mXuid}, {"name", mName}}},
        {"minecraft:custom",    mData->custom                                                 },
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
void PlayerStats::addStats(StatsType type, std::string key, uint64_t value) {
    if (value <= 0) return;
    switch (type) {
    case StatsType::custom:
        mData->custom[key] += value;
        break;
    case StatsType::mined:
        mData->mined[key] += value;
        break;
    case StatsType::broken:
        mData->broken[key] += value;
        break;
    case StatsType::crafted:
        mData->crafted[key] += value;
        break;
    case StatsType::used:
        mData->used[key] += value;
        break;
    case StatsType::picked_up:
        mData->picked_up[key] += value;
        break;
    case StatsType::dropped:
        mData->dropped[key] += value;
        break;
    case StatsType::killed:
        mData->killed[key] += value;
        break;
    case StatsType::killed_by:
        mData->killed_by[key] += value;
        break;
    }
    auto typeName = StatsTypeMap.at(type);
    // getLogger().debug("AddStats {} {} key:{} value:{}", mName, typeName, key, value);
};
void PlayerStats::addCustomStats(CustomType type, uint64_t value) {
    if (value <= 0) return;
    auto key = CustomTypeMap.at(type);
    // getLogger().debug("AddCustomStats {} key:{} value:{}", mName, key, value);

    mData->custom[key] += value;
};

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