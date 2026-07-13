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

inline void renderContent(std::string& content, std::vector<StatsPair> const& data) {
    content.reserve(content.size() + data.size() * 32);
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string str  = "§6" + it->first + "§r : §a" + std::to_string(it->second) + "§r\n";
        content         += str;
    }
}

struct StatsView {
    StatsDataMap const* data;
    uint64_t            playTimeDelta;
};

inline StatsView getStatsView(mce::UUID uuid, StatsType type) {
    if (auto* playerStats = findPlayerStats(uuid)) {
        auto const level = ll::service::getLevel();
        auto const tick  = level ? level->getCurrentTick().tickID : 0;
        return {playerStats->getStatsMap(type), playerStats->getPendingPlayTime(tick)};
    }

    auto const* cached = findCachedStats(uuid);
    return {cached && cached->second ? cached->second->getMap(type) : nullptr, 0};
}

std::optional<query::StatsEntries> buildStatsEntries(mce::UUID uuid, StatsType type) {
    auto const view = getStatsView(uuid, type);
    if (!view.data) return std::nullopt;

    auto dataVector = query::buildDisplayEntries(*view.data, type, view.playTimeDelta);
    if (type == StatsType::custom) {
        auto const level = ll::service::getLevel();
        if (level) {
            dataVector.emplace_back(
                "minecraft:total_world_time",
                level->getCurrentServerTick().tickID
            );
        }
    }
    for (auto& pair : dataVector) {
        pair.first = std::string(ll::i18n::getInstance().get(pair.first, {}));
    }
    query::sortStatsEntries(dataVector, type == StatsType::custom);
    return dataVector;
}

std::string pageSuffix(query::StatsPage const& page) {
    auto const current = page.totalPages == 0 ? 0 : page.pageIndex + 1;
    return " [" + std::to_string(current) + "/" + std::to_string(page.totalPages) + "]";
}

std::optional<std::string> renderStatsContent(mce::UUID uuid, StatsType type) {
    auto entries = buildStatsEntries(uuid, type);
    if (!entries) return std::nullopt;

    std::string content;
    renderContent(content, *entries);
    return content;
}

void sendStatsGui(Player& player, StatsType type, std::size_t pageIndex) {
    sendStatsGui(player, player.getUuid(), {}, type, pageIndex);
}

void sendStatsGui(
    Player&     player,
    mce::UUID   targetUuid,
    std::string targetName,
    StatsType   type,
    std::size_t pageIndex
) {
    auto entries = buildStatsEntries(targetUuid, type);
    if (!entries) return;

    auto page       = query::paginate(std::move(*entries), pageIndex);
    auto typeString = getStatsTypeKey(type);
    auto title      = "gui.title.stats"_tr() + " | " + std::string(ll::i18n::getInstance().get(typeString, {}));
    if (!targetName.empty()) title += " | " + targetName;
    title += pageSuffix(page);

    std::string content;
    renderContent(content, page.entries);
    auto fm = ll::form::SimpleForm(title, content);
    if (page.pageIndex > 0) {
        fm.appendButton("<", [targetUuid, targetName, type, pageIndex = page.pageIndex - 1](Player& viewer) {
            sendStatsGui(viewer, targetUuid, targetName, type, pageIndex);
        });
    }
    if (page.pageIndex + 1 < page.totalPages) {
        fm.appendButton(">", [targetUuid, targetName, type, pageIndex = page.pageIndex + 1](Player& viewer) {
            sendStatsGui(viewer, targetUuid, targetName, type, pageIndex);
        });
    }
    fm.sendTo(player);
}

void sendRankGui(Player& player, StatsType statsType, std::string type, std::size_t pageIndex) {
    auto        page    = getStatsRankPage(statsType, type, pageIndex);
    std::string title   = type.empty() ? "" : " | " + std::string(ll::i18n::getInstance().get(type, {}));
    std::string content;
    renderContent(content, page.entries);
    auto typeString = getStatsTypeKey(statsType);
    auto fm         = ll::form::SimpleForm();
    fm.setTitle(
          "gui.title.rank"_tr() + " | " + std::string(ll::i18n::getInstance().get(typeString, {})) + title
          + pageSuffix(page)
    ).setContent(content);
    if (page.pageIndex > 0) {
        fm.appendButton("<", [statsType, type, pageIndex = page.pageIndex - 1](Player& viewer) {
            sendRankGui(viewer, statsType, type, pageIndex);
        });
    }
    if (page.pageIndex + 1 < page.totalPages) {
        fm.appendButton(">", [statsType, type, pageIndex = page.pageIndex + 1](Player& viewer) {
            sendRankGui(viewer, statsType, type, pageIndex);
        });
    }
    fm.sendTo(player);
}

} // namespace stats::form
