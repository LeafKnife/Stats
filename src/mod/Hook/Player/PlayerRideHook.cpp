#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>

#include "mod/Events/PlayerEventHandle.h"

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
    event::player::onStartRiding(getUuid(), vehicle, forceRiding);
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
    if (!hasType(::ActorType::Player))
        return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
    Player* player = getWeakEntity().tryUnwrap<Player>();
    if (!player) return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
    event::player::onStopRiding(player->getUuid(), player->getVehicle());
    origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles, isBeingTeleported);
}

void hookPlayerRiding() {
    PlayerStartRiding::hook();
    ActorStopRidingHook::hook();
}
} // namespace stats::hook::player