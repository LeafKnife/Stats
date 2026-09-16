#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    ServerPlayerOpenTradingHook,
    HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$openTrading,
    void,
    ::ActorUniqueID const& uniqueID,
    bool                   useNewScreen
) {
    origin(uniqueID, useNewScreen);
    Player* player = this;
    auto level = ll::service::getLevel();
    if (!level) return;
    auto actor = level->fetchEntity(uniqueID, false);

    if (!actor || !actor->isType(::ActorType::VillagerV2)) return;
    handler::onPlayerOpenTrading(player);
}

void hookServerPlayerOpenTrading() { ServerPlayerOpenTradingHook::hook(); }
void unhookServerPlayerOpenTrading() { ServerPlayerOpenTradingHook::unhook(); }
} // namespace stats::hook::player
