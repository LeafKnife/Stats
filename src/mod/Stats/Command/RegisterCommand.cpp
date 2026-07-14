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
    StatsType   StatsType;
    std::string playerName;
};

struct StatsRank {
    StatsType   StatsType;
    std::string type;
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
        .optional("StatsType")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsGui const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.notplayer"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            // lk::MyMod::getInstance().getSelf().getLogger().info("cmd {} {}", player->getRealName(), param.statsType);
            if (isValidStatsType(param.StatsType)) {
                form::sendStatsGui(*player, param.StatsType);
            } else {
                form::sendMainGui(*player);
            }
        });

    cmd.overload<StatsGui>()
        .text("player")
        .required("playerName")
        .required("StatsType")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsGui const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.notplayer"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            if (param.playerName.empty()) {
                return output.error("command.error.player_name_empty"_tr());
            }
            auto const* cached = findCachedStatsByName(param.playerName);
            if (!cached) return output.error("command.error.find_player"_tr());
            form::sendStatsGui(
                *player,
                mce::UUID(cached->first.uuid),
                param.playerName,
                param.StatsType
            );
        });

    cmd.overload<StatsRank>()
        .text("rank")
        .required("StatsType")
        .optional("type")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsRank const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(::ActorType::Player)) {
                return output.error("command.error.notplayer"_tr());
            }
            auto* player = static_cast<Player*>(entity);
            if (!isValidStatsType(param.StatsType)) {
                return output.error("command.error.invalid_stats_type"_tr());
            }
            if (param.StatsType == StatsType::custom && param.type.empty()) {
                return output.error("command.error.rank_type_required"_tr());
            }
            form::sendRankGui(*player, param.StatsType, param.type);
        });
}
} // namespace stats::command
