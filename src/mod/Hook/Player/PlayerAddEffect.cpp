#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/effect/MobEffectInstance.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {

LL_TYPE_INSTANCE_HOOK(
    ActorAddEffectHook,
    HookPriority::Normal,
    Actor,
    &Actor::addEffect,
    void,
    ::MobEffectInstance const& effect
) {
    if (!this->isType(::ActorType::Player)) return origin(effect);
    Player* player = this->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return origin(effect);
    handler::onPlayerEffectAdded(player, effect);
    return origin(effect);
}

void hookPlayerAddEffect() { ActorAddEffectHook::hook(); }
void unhookPlayerAddEffect() { ActorAddEffectHook::unhook(); }
} // namespace stats::hook::player
