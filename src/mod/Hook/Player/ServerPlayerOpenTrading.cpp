#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/level/Level.h>

#include "mod/Events/PlayerEventHandle.h"

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
    auto actor = ll::service::getLevel()->fetchEntity(uniqueID, false);

    if (!actor->hasType(::ActorType::VillagerV2)) return;
    event::player::onOpenTrading(player);
}

void hookServerPlayerOpenTrading() { ServerPlayerOpenTradingHook::hook(); }
} // namespace stats::hook::player