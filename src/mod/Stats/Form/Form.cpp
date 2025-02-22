#include "mod/Stats/Form/Form.h"

#include "mod/Stats/Stats.h"
#include <cstdint>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>
#include <memory>
#include <mod/Stats/StatsType.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>


using namespace ll::i18n_literals;

namespace stats::form {
namespace {
typedef std::pair<std::string, uint64_t> StatsPair;
} // namespace
void sendMainGui(Player& player) {
    auto fm = ll::form::SimpleForm();
    fm.setTitle("gui.title.stats"_tr())
        .appendButton("minecraft:custom"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::custom); })
        .appendButton("minecraft:mined"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::mined); })
        .appendButton("minecraft:broken"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::broken); })
        .appendButton("minecraft:crafted"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::crafted); })
        .appendButton("minecraft:used"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::used); })
        .appendButton("minecraft:dropped"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::dropped); })
        .appendButton("minecraft:picked_up"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::picked_up); })
        .appendButton("minecraft:killed"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::killed); })
        .appendButton("minecraft:killed_by"_tr(), [](Player& pl) { sendStatsGui(pl, StatsDataType::killed_by); })
        .sendTo(player);
}

void sendStatsGui(Player& player, StatsDataType type) {
    auto  uuid           = player.getUuid();
    auto& playerStatsMap = getPlayerStatsMap();
    auto  findPlayer     = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    auto                   j          = playerStats->getJson();
    auto                   typeString = StatsDataTypeMap.at(type);
    std::string            content    = "";
    auto                   dataMap    = j[typeString].get<StatsDataMap>();
    std::vector<StatsPair> dataVector;
    if (type != StatsDataType::custom) {
        for (const auto& pair : dataMap) {
            dataVector.push_back(std::make_pair(pair.first, pair.second));
        }
        std::sort(dataVector.begin(), dataVector.end(), [](const StatsPair& a, const StatsPair& b) {
            return a.second > b.second;
        });
    } else {
        auto playtime                  = dataMap["minecraft:play_time"];
        dataMap["minecraft:play_time"] = playtime + player.mTickCount;
        for (const auto& pair : dataMap) {
            // if (pair.first == "minecraft:play_time") pair.second += player.mTickCount;
            dataVector.push_back(std::make_pair(pair.first, pair.second));
        }
        auto levelTick =
            std::make_pair("minecraft:total_world_time", ll::service::getLevel()->getCurrentServerTick().tickID);
        dataVector.push_back(levelTick);
        std::sort(dataVector.begin(), dataVector.end(), [](const StatsPair& a, const StatsPair& b) {
            return a.first < b.first;
        });
    }
    for (auto it = dataVector.begin(); it != dataVector.end(); ++it) {

        std::string str = "§6" + std::string(ll::i18n::getInstance().get(it->first, {})) + "§r : §a"
                        + std::to_string(it->second) + "§r\n";
        content += str;
    }
    auto fm = ll::form::SimpleForm();
    fm.setTitle("gui.title.stats"_tr() + " - " + std::string(ll::i18n::getInstance().get(typeString, {})))
        .setContent(content)
        .sendTo(player);
}

inline uint64_t getStatsDataMapValue(StatsDataMap const& map, std::string type) {
    uint64_t value;
    if (type.empty()) {
        value =
            std::accumulate(map.begin(), map.end(), 0, [](int total, const StatsPair& p) { return total + p.second; });
    } else {
        auto mapValue = map.find(type);
        if (mapValue != map.end()) {
            value = mapValue->second;
        } else {
            value = 0;
        }
    }
    return value;
}

void sendRankGui(Player& player, StatsDataType statsType, std::string type) {
    auto                   cache = getStatsCache();
    std::vector<StatsPair> data;
    switch (statsType) {
    case StatsDataType::custom:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->custom;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::broken:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->broken;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::used:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->used;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::dropped:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->dropped;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::picked_up:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->picked_up;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::crafted:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->crafted;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::mined:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->mined;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::killed:
        for (auto it : cache) {
            auto const& map   = it.second->killed;
            auto        name  = it.first.name;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    case StatsDataType::killed_by:
        for (auto it : cache) {
            auto        name  = it.first.name;
            auto const& map   = it.second->killed_by;
            auto        value = getStatsDataMapValue(map, type);
            data.push_back(std::make_pair(name, value));
        };
        break;
    default:
        break;
    }
    std::sort(data.begin(), data.end(), [](const StatsPair& a, const StatsPair& b) { return a.second > b.second; });
    std::string content = type.empty() ? "" : std::string(ll::i18n::getInstance().get(type, {})) + "\n";
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string str  = "§6" + it->first + "§r : §a" + std::to_string(it->second) + "§r\n";
        content         += str;
    }
    auto typeString = StatsDataTypeMap.at(statsType);
    auto fm         = ll::form::SimpleForm();
    fm.setTitle("gui.title.rank"_tr() + " - " + std::string(ll::i18n::getInstance().get(typeString, {})))
        .setContent(content)
        .sendTo(player);
}

} // namespace stats::form