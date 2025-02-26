#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <stdexcept>

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
    getLogger()
        .info("StartRidingAfter {} {} {} {}", r, getRealName(), vehicle.getOrCreateUniqueID().rawID, forceRiding);
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
    bool switchingVehicles
) {
    if (!hasType(::ActorType::Player)) return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles);
    Player* player = getWeakEntity().tryUnwrap<Player>();
    if (!player) return origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles);
    event::player::onStopRiding(player->getUuid(), *player->getVehicle());
    origin(exitFromPassenger, actorIsBeingDestroyed, switchingVehicles);
}

void hookPlayerRiding() {
    PlayerStartRiding::hook();
    ActorStopRidingHook::hook();
}
} // namespace stats::hook::player