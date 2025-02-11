#include "mod/Stats/Form/Form.h"

#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"
#include "nlohmann/json.hpp"

#include "mod/Stats/Stats.h"
#include <string>
#include <string_view>

using namespace ll::i18n_literals;

namespace stats::form {
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
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    auto        j          = playerStats->getJson();
    auto        typeString = StatsDataTypeMap.at(type);
    std::string content    = "";
    auto        data       = j[typeString].get<StatsDataMap>();
    for (auto it = data.begin(); it != data.end(); ++it) {

        std::string str = "§6" + std::string(ll::i18n::getInstance().get(it->first, {})) + "§r : §a"
                        + std::to_string(it->second) + "§r\n";
        content += str;
    }
    auto fm = ll::form::SimpleForm();
    fm.setTitle("gui.title.stats"_tr() + " - " + std::string(ll::i18n::getInstance().get(typeString, {})))
        .setContent(content)
        .sendTo(player);
}

} // namespace stats::form