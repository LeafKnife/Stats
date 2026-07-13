#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {

LL_TYPE_INSTANCE_HOOK(
    PlayerStartRiding,
    HookPriority::Normal,
    Player,
    &Player::$startRiding,
    bool,
    ::Actor& vehicle,
    bool     forceRiding
) {
    auto r = origin(vehicle, forceRiding);
    if (!r) return r;
    handler::onPlayerStartRiding(getUuid());
    return r;
}

LL_TYPE_INSTANCE_HOOK(
    ActorStopRidingHook,
    HookPriority::Normal,
    Actor,
    &Actor::stopRiding,
    void,
    bool exitFromPassenger,
    bool actorIsBeingDestroyed,
    bool switchingVehicles,
    bool isBeingTeleported
) {
    if (!isType(::ActorType::Player))
        return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
    Player* player = getWeakEntity().tryUnwrap<Player>();
    if (!player) return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
    handler::onPlayerStopRiding(player->getUuid(), player->getVehicle());
    origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
}

void hookPlayerRiding() {
    PlayerStartRiding::hook();
    ActorStopRidingHook::hook();
}
void unhookPlayerRiding() {
    ActorStopRidingHook::unhook();
    PlayerStartRiding::unhook();
}
} // namespace stats::hook::player
