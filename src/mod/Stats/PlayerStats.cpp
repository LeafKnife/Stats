#include "mod/Stats/PlayerStats.h"

#include "ll/api/io/FileUtils.h"

#include "mod/Stats/Stats.h"

#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>


namespace stats {

PlayerStats::PlayerStats(Player const& player) {
    mUuid = player.getUuid();
    mXuid = player.getXuid();
    mName = player.getRealName();

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
void PlayerStats::addStats(StatsDataType type, std::string key, int value) {
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
void PlayerStats::addCustomStats(CustomType type, int value) {
    auto key            = CustomTypeMap.at(type);
    mData->custom[key] += value;
};
} // namespace stats