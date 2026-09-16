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

std::string translateStatsCategory(StatsType type);

void sendMainGui(Player& player) {
    auto fm = ll::form::SimpleForm();
    fm.setTitle("gui.title.stats"_tr())
        .appendButton(translateStatsCategory(StatsType::custom), [](Player& pl) { sendStatsGui(pl, StatsType::custom); })
        .appendButton(translateStatsCategory(StatsType::mined), [](Player& pl) { sendStatsGui(pl, StatsType::mined); })
        .appendButton(translateStatsCategory(StatsType::broken), [](Player& pl) { sendStatsGui(pl, StatsType::broken); })
        .appendButton(translateStatsCategory(StatsType::crafted), [](Player& pl) { sendStatsGui(pl, StatsType::crafted); })
        .appendButton(translateStatsCategory(StatsType::used), [](Player& pl) { sendStatsGui(pl, StatsType::used); })
        .appendButton(translateStatsCategory(StatsType::dropped), [](Player& pl) { sendStatsGui(pl, StatsType::dropped); })
        .appendButton(translateStatsCategory(StatsType::picked_up), [](Player& pl) { sendStatsGui(pl, StatsType::picked_up); })
        .appendButton(translateStatsCategory(StatsType::killed), [](Player& pl) { sendStatsGui(pl, StatsType::killed); })
        .appendButton(translateStatsCategory(StatsType::killed_by), [](Player& pl) { sendStatsGui(pl, StatsType::killed_by); })
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

std::string translateStatsCategory(StatsType type) {
    auto& i18n = ll::i18n::getInstance();
    auto const translationKey = getStatsCategoryTranslationKey(type);
    auto const translation    = i18n.get(translationKey, {});
    if (hasTranslation(translation, translationKey)) return std::string(translation);

    // Keep external language files written for the former flat key layout working.
    auto const legacyKey         = getStatsTypeKey(type);
    auto const legacyTranslation = i18n.get(legacyKey, {});
    return hasTranslation(legacyTranslation, legacyKey) ? std::string(legacyTranslation) : std::string(legacyKey);
}

std::string translateStatsEntry(StatsType type, std::string_view typeName) {
    auto& i18n = ll::i18n::getInstance();
    auto translate = [&i18n](std::string const& key) -> std::optional<std::string> {
        auto const translation = i18n.get(key, {});
        if (!hasTranslation(translation, key)) return std::nullopt;
        return std::string(translation);
    };

    if (isBlockOrItemStatsCategory(type)) {
        if (auto translation = translate("item." + std::string(typeName))) return *translation;
        if (auto translation = translate("block." + std::string(typeName))) return *translation;
    } else if (auto translation = translate(getStatsEntryTranslationKey(type, typeName))) {
        return *translation;
    }

    // Older custom language files use the bare Minecraft type name.
    auto const legacyTranslation = i18n.get(typeName, {});
    return hasTranslation(legacyTranslation, typeName) ? std::string(legacyTranslation) : std::string(typeName);
}

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
        pair.first = translateStatsEntry(type, pair.first);
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

    auto page  = query::paginate(std::move(*entries), pageIndex);
    auto title = "gui.title.stats"_tr() + " | " + translateStatsCategory(type);
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
    std::string title   = type.empty() ? "" : " | " + translateStatsEntry(statsType, type);
    std::string content;
    renderContent(content, page.entries);
    auto fm = ll::form::SimpleForm();
    fm.setTitle(
          "gui.title.rank"_tr() + " | " + translateStatsCategory(statsType) + title
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
