#include "mod/Stats/Command/RegisterCommand.h"

#include <optional>
#include <string>

#include <ll/api/Expected.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <ll/api/form/SimpleForm.h>
#include <mc/platform/UUID.h>

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
    auto& cmd = ll::command::CommandRegistrar::getInstance()
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
            if (entity == nullptr || !entity->hasType(::ActorType::Player)) {
                output.error("command.error.notplayer"_tr());
            }
            Player* player = (Player*)entity;
            // lk::MyMod::getInstance().getSelf().getLogger().info("cmd {} {}", player->getRealName(), param.statsType);
            switch (param.StatsType) {
            case StatsType::custom:
            case StatsType::mined:
            case StatsType::broken:
            case StatsType::crafted:
            case StatsType::used:
            case StatsType::picked_up:
            case StatsType::dropped:
            case StatsType::killed:
            case StatsType::killed_by:
                form::sendStatsGui(*player, param.StatsType);
                break;
            default:
                form::sendMainGui(*player);
                break;
            }
        });

    cmd.overload<StatsGui>()
        .text("player")
        .required("playerName")
        .required("StatsType")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsGui const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->hasType(::ActorType::Player)) {
                output.error("command.error.notplayer"_tr());
            }
            Player* player = (Player*)entity;
            if (param.playerName.empty()) {
                return output.error("command.error.player_name_empty"_tr());
            }
            // auto playerInfo = ll::service::PlayerInfo::getInstance().fromName(param.playerName);
            // if (!playerInfo.has_value()) return output.error("Error");
            // auto                       uuid    = playerInfo->uuid;
            auto const& cache = getStatsCache();
            std::string uuid;
            for (auto& it : cache) {
                if (it.first.name == param.playerName) {
                    uuid = it.first.uuid;
                    break;
                }
            }
            std::optional<std::string> content = form::renderStatsContent(uuid, param.StatsType);
            if (!content.has_value()) return output.error("command.error.find_player"_tr());
            auto fm         = ll::form::SimpleForm();
            auto typeString = StatsTypeMap.at(param.StatsType);
            fm.setTitle(
                  "gui.title.stats"_tr() + " | " + std::string(ll::i18n::getInstance().get(typeString, {})) + " | "
                  + param.playerName
            )
                .setContent(content.value())
                .sendTo(*player);
        });

    cmd.overload<StatsRank>()
        .text("rank")
        .required("StatsType")
        .optional("type")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, StatsRank const& param) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->hasType(::ActorType::Player)) {
                output.error("command.error.notplayer"_tr());
            }
            Player* player = (Player*)entity;
            switch (param.StatsType) {
            case StatsType::custom:
                if (param.type.empty()) {
                    output.error("command.error.rank_type_required"_tr());
                } else {
                    form::sendRankGui(*player, StatsType::custom, param.type);
                }
                break;
            case StatsType::mined:
            case StatsType::broken:
            case StatsType::crafted:
            case StatsType::used:
            case StatsType::picked_up:
            case StatsType::dropped:
            case StatsType::killed:
            case StatsType::killed_by:
                form::sendRankGui(*player, param.StatsType, param.type);
                break;
            default:
                // form::sendRankGuiMain(*player);
                output.error("TODO");
                break;
            }
        });
}
} // namespace stats::command