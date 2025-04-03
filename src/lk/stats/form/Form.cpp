#include "lk/stats/Form/Form.h"

#include "mc/platform/UUID.h"
#include "lk/stats/Stats.h"

#include <cstdint>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


using namespace ll::i18n_literals;

namespace stats::form {

void sendMainGui(Player& player) {
    auto fm = ll::form::SimpleForm();
    fm.setTitle("gui.title.stats"_tr())
        .appendButton("minecraft:custom"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::custom); })
        .appendButton("minecraft:mined"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::mined); })
        .appendButton("minecraft:broken"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::broken); })
        .appendButton("minecraft:crafted"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::crafted); })
        .appendButton("minecraft:used"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::used); })
        .appendButton("minecraft:dropped"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::dropped); })
        .appendButton("minecraft:picked_up"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::picked_up); })
        .appendButton("minecraft:killed"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::killed); })
        .appendButton("minecraft:killed_by"_tr(), [](Player& pl) { sendStatsGui(pl, StatsType::killed_by); })
        .sendTo(player);
}

void sendStatsGui(Player& player, StatsType type) {
    auto content = renderStatsContent(player.getUuid(), type, player.mTickCount);
    if (content.has_value()) {
        auto fm         = ll::form::SimpleForm();
        auto typeString = StatsTypeMap.at(type);
        fm.setTitle("gui.title.stats"_tr() + " - " + std::string(ll::i18n::getInstance().get(typeString, {})))
            .setContent(content.value())
            .sendTo(player);
    }
};

inline void renderContent(std::string& content, std::vector<StatsPair> const& data) {
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string str  = "§6" + it->first + "§r : §a" + std::to_string(it->second) + "§r\n";
        content         += str;
    }
}

inline std::optional<StatsDataMap> getStatsDataMap(mce::UUID uuid, StatsType type) {
    auto& playerStatsMap = getPlayerStatsMap();
    auto  findPlayer     = playerStatsMap.find(uuid);
    if (findPlayer != playerStatsMap.end()) {
        auto playerStats = findPlayer->second;
        if (!playerStats) return std::nullopt;
        auto j          = playerStats->getJson();
        auto typeString = StatsTypeMap.at(type);
        auto dataMap    = j[typeString].get<StatsDataMap>();
        return std::make_optional(dataMap);
    } else {
        auto& cache = getStatsCache();
        for (auto it : cache) {
            if (it.first.uuid != uuid) continue;
            if (auto r = it.second->getMap(type); r) {
                return std::make_optional(*r);
            } else {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
}

std::optional<std::string> renderStatsContent(mce::UUID uuid, StatsType type, uint64_t tick) {
    auto data = getStatsDataMap(uuid, type);

    if (!data.has_value()) return std::nullopt;

    auto& dataMap = data.value();

    std::vector<StatsPair> dataVector;
    for (const auto& pair : dataMap) {
        dataVector.push_back(std::make_pair(std::string(ll::i18n::getInstance().get(pair.first, {})), pair.second));
    }
    if (type == StatsType::custom) {
        dataMap["minecraft:play_time"] += tick;
        auto levelTick =
            std::make_pair("minecraft:total_world_time"_tr(), ll::service::getLevel()->getCurrentServerTick().tickID);
        dataVector.push_back(levelTick);
        std::sort(dataVector.begin(), dataVector.end(), [](const StatsPair& a, const StatsPair& b) {
            return a.first < b.first;
        });
    } else {
        std::sort(dataVector.begin(), dataVector.end(), [](const StatsPair& a, const StatsPair& b) {
            return a.second > b.second;
        });
    }

    std::string content = "";
    renderContent(content, dataVector);
    return content;
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

inline void getRankData(std::vector<StatsPair>& data, StatsType statsType, std::string type) {
    auto& cache = getStatsCache();
    for (auto it : cache) {
        auto name  = it.first.name;
        auto map   = it.second->getMap(statsType);
        auto value = getStatsDataMapValue(*map, type);
        data.push_back(std::make_pair(name, value));
    };
    std::sort(data.begin(), data.end(), [](const StatsPair& a, const StatsPair& b) { return a.second > b.second; });
}

void sendRankGui(Player& player, StatsType statsType, std::string type) {
    std::vector<StatsPair> data;
    getRankData(data, statsType, type);
    std::string title   = type.empty() ? "" : " | " + std::string(ll::i18n::getInstance().get(type, {}));
    std::string content = "";
    renderContent(content, data);
    auto typeString = StatsTypeMap.at(statsType);
    auto fm         = ll::form::SimpleForm();
    fm.setTitle("gui.title.rank"_tr() + " | " + std::string(ll::i18n::getInstance().get(typeString, {})) + title)
        .setContent(content)
        .sendTo(player);
}

} // namespace stats::form