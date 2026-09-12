#include "mod/Stats/Command/RegisterCommand.h"

#include <string>

#include <ll/api/Expected.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/form/SimpleForm.h>
#include <ll/api/i18n/I18n.h>
#include <mc/platform/UUID.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>


#include "mod/Stats/Form/Form.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsData.h"

using namespace ll::i18n_literals;

namespace stats::command {
struct StatsGui {
    StatsType   category;
    std::string playerName;
};

struct StatsRank {
    StatsType   category;
    std::string stat;
};

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                    .getOrCreateCommand(
                        "command.stats.name"_tr(),
                        "LK-Stats - " + "command.stats.desc"_tr(),
                        CommandPermissionLevel::Any
                    )
                    .alias("command.stats.alias"_tr());
    cmd.overload<StatsGui>()
        .text("gui")
        .optional("category")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsGui const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.not_player"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            // lk::MyMod::getInstance().getSelf().getLogger().info("cmd {} {}", player->getRealName(), param.category);
            if (isValidStatsType(param.category)) {
                form::sendStatsGui(*player, param.category);
            } else {
                form::sendMainGui(*player);
            }
        });

    cmd.overload<StatsGui>()
        .text("player")
        .required("playerName")
        .required("category")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsGui const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.not_player"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            if (param.playerName.empty()) {
                return output.error("command.error.player_name_empty"_tr());
            }
            auto const* cached = findCachedStatsByName(param.playerName);
            if (!cached) return output.error("command.error.player_not_found"_tr());
            form::sendStatsGui(
                *player,
                mce::UUID(cached->first.uuid),
                param.playerName,
                param.category
            );
        });

    cmd.overload<StatsRank>()
        .text("rank")
        .required("category")
        .optional("stat")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsRank const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.not_player"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            if (!isValidStatsType(param.category)) {
                return output.error("command.error.invalid_stats_category"_tr());
            }
            if (param.category == StatsType::custom && param.stat.empty()) {
                return output.error("command.error.rank_stat_required"_tr());
            }
            form::sendRankGui(*player, param.category, param.stat);
        });
}
} // namespace stats::command
