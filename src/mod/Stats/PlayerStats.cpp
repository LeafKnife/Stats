#include "mod/Stats/PlayerStats.h"

#include "ll/api/io/FileUtils.h"

#include "nlohmann/json.hpp"

#include <string>


namespace stats {

PlayerStats::PlayerStats(Player const& player) {
    mUuid     = player.getUuid();
    mXuid     = player.getXuid();
    mName     = player.getName();
    auto data = ll::file_utils::readFile(getPath());
    if (data && !data->empty()) {
        parseData(*data);
    } else {
        mData = StatsData();
        saveData();
    }
};
void PlayerStats::parseData(std::string const& data) {
    auto j          = nlohmann::json::parse(data);
    mData.custom    = j["minecraft:custom"];
    mData.mined     = j["minecraft:mined"];
    mData.broken    = j["minecraft:broken"];
    mData.crafted   = j["minecraft:crafted"];
    mData.used      = j["minecraft:used"];
    mData.picked_up = j["minecraft:picked_up"];
    mData.dropped   = j["minecraft:dropped"];
    mData.killed    = j["minecraft:killed"];
    mData.killed_by = j["minecraft:killed_by"];
};
mce::UUID             PlayerStats::getUuid() { return mUuid; };
std::filesystem::path PlayerStats::getPath() { return ll::file_utils::u8path("stats/" + mUuid.asString() + ".json"); };
nlohmann::json        PlayerStats::getJson() {
    nlohmann::json j = {
        {"playerInfo",          {{"uuid", mUuid.asString()}, {"xuid", mXuid}, {"name", mName}}},
        {"minecraft:custom",    mData.custom                                                  },
        {"minecraft:mined",     mData.mined                                                   },
        {"minecraft:broken",    mData.broken                                                  },
        {"minecraft:crafted",   mData.crafted                                                 },
        {"minecraft:used",      mData.used                                                    },
        {"minecraft:picked_up", mData.picked_up                                               },
        {"minecraft:dropped",   mData.dropped                                                 },
        {"minecraft:killed",    mData.killed                                                  },
        {"minecraft:killed_by", mData.killed_by                                               },
    };
    return j;
}
// nlohmann::json PlayerStats::getJsonStatsData(StatsDataType type) {
//     nlohmann::json j = nlohmann::json::object();
//     switch (type) {
//     case StatsDataType::custom:
//         j = mData.custom;
//         break;
//     case StatsDataType::broken:
//         j = mData.broken;
//         break;
//     case StatsDataType::crafted:
//         j = mData.crafted;
//         break;
//     case StatsDataType::dropped:
//         j = mData.dropped;
//         break;
//     case StatsDataType::picked_up:
//         j = mData.picked_up;
//         break;
//     case StatsDataType::mined:
//         j = mData.mined;
//         break;
//     case StatsDataType::used:
//         j = mData.used;
//         break;
//     case StatsDataType::killed:
//         j = mData.killed;
//         break;
//     case StatsDataType::killed_by:
//         j = mData.killed_by;
//         break;
//     }
//     return j;
// }
bool PlayerStats::saveData() {
    auto j = getJson();
    return ll::file_utils::writeFile(getPath(), j.dump());
};
void PlayerStats::addStats(StatsDataType type, std::string key, int value) {
    switch (type) {
    case StatsDataType::custom:
        mData.custom[key] += value;
        break;
    case StatsDataType::mined:
        mData.mined[key] += value;
        break;
    case StatsDataType::broken:
        mData.broken[key] += value;
        break;
    case StatsDataType::crafted:
        mData.crafted[key] += value;
        break;
    case StatsDataType::used:
        mData.used[key] += value;
        break;
    case StatsDataType::picked_up:
        mData.picked_up[key] += value;
        break;
    case StatsDataType::dropped:
        mData.dropped[key] += value;
        break;
    case StatsDataType::killed:
        mData.killed[key] += value;
        break;
    case StatsDataType::killed_by:
        mData.killed_by[key] += value;
        break;
    }
};
void PlayerStats::addCustomStats(CustomType type, int value) {
    auto key           = CustomTypeMap.at(type);
    mData.custom[key] += value;
};
} // namespace stats