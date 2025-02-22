#include "mod/Stats/PlayerStats.h"

#include "ll/api/io/FileUtils.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"

#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsType.h"

#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>


namespace stats {

PlayerStats::PlayerStats(Player const& player) {
    mUuid              = player.getUuid();
    mXuid              = player.getXuid();
    mName              = player.getRealName();
    mSneakingStartTick = 0;

    auto                 cache = getStatsCache();
    auto                 uuid  = mUuid.asString();
    StatsCache::iterator it    = std::find_if(cache.begin(), cache.end(), [&uuid](const StatsCacheData& data) {
        return data.first.uuid.compare(uuid) == 0;
    });
    if (it != cache.end()) {
        mData = it->second;
    } else {
        mData           = std::make_shared<StatsData>();
        PlayerInfo info = {mUuid.asString(), mXuid, mName};
        cache.push_back(std::make_pair(info, mData));
        saveData();
    }
};
mce::UUID             PlayerStats::getUuid() { return mUuid; };
std::filesystem::path PlayerStats::getPath() { return ll::file_utils::u8path("stats/" + mUuid.asString() + ".json"); };
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
    auto j = getJson();
    return ll::file_utils::writeFile(getPath(), j.dump());
};
void PlayerStats::addStats(StatsDataType type, std::string key, uint64_t value) {
    switch (type) {
    case StatsDataType::custom:
        mData->custom[key] += value;
        break;
    case StatsDataType::mined:
        mData->mined[key] += value;
        break;
    case StatsDataType::broken:
        mData->broken[key] += value;
        break;
    case StatsDataType::crafted:
        mData->crafted[key] += value;
        break;
    case StatsDataType::used:
        mData->used[key] += value;
        break;
    case StatsDataType::picked_up:
        mData->picked_up[key] += value;
        break;
    case StatsDataType::dropped:
        mData->dropped[key] += value;
        break;
    case StatsDataType::killed:
        mData->killed[key] += value;
        break;
    case StatsDataType::killed_by:
        mData->killed_by[key] += value;
        break;
    }
};
void PlayerStats::addCustomStats(CustomType type, uint64_t value) {
    auto key            = CustomTypeMap.at(type);
    mData->custom[key] += value;
};

void PlayerStats::startSneaking() { mSneakingStartTick = ll::service::getLevel()->getCurrentTick().tickID; };
void PlayerStats::addSneakTick() {
    if (mSneakingStartTick == 0) return;
    auto record = ll::service::getLevel()->getCurrentTick().tickID - mSneakingStartTick;
    addCustomStats(CustomType::sneak_time, record);
    mSneakingStartTick = 0;
};
} // namespace stats