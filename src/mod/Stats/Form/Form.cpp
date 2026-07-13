#include "mod/Stats/Form/Form.h"

#include "mc/platform/UUID.h"
#include "mod/Stats/Stats.h"

#include <algorithm>
#include <cstdint>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>
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
    content.reserve(content.size() + data.size() * 32);
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string str  = "§6" + it->first + "§r : §a" + std::to_string(it->second) + "§r\n";
        content         += str;
    }
}

inline std::optional<StatsDataMap> getStatsDataMap(mce::UUID uuid, StatsType type) {
    if (auto* playerStats = findPlayerStats(uuid)) {
        if (auto const* dataMap = playerStats->getStatsMap(type)) {
            return *dataMap;
        }
        return std::nullopt;
    } else {
        auto& cache = getStatsCache();
        for (auto const& it : cache) {
            if (mce::UUID(it.first.uuid) != uuid) continue;
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

    if (type == StatsType::custom) {
        dataMap["minecraft:play_time"] += tick;
    }

    std::vector<StatsPair> dataVector;
    dataVector.reserve(dataMap.size() + (type == StatsType::custom ? 1 : 0));
    for (const auto& pair : dataMap) {
        dataVector.push_back(std::make_pair(std::string(ll::i18n::getInstance().get(pair.first, {})), pair.second));
    }
    if (type == StatsType::custom) {
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

void sendRankGui(Player& player, StatsType statsType, std::string type) {
    auto        data    = getStatsRank(statsType, type);
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
