#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/deps/ecs/WeakEntityRef.h"

#include "mod/Events/PlayerEventHandle.h"

namespace stats::hook::player {

LL_TYPE_INSTANCE_HOOK(
    ActorAddEffectHook,
    HookPriority::Normal,
    Actor,
    &Actor::addEffect,
    void,
    ::MobEffectInstance const& effect
) {
    if (!this->hasType(::ActorType::Player)) return origin(effect);
    Player* player = this->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return origin(effect);
    event::player::onEffectAdded(player, effect);
    return origin(effect);
}

void hookPlayerAddEffect() { ActorAddEffectHook::hook(); }
} // namespace stats::hook::player