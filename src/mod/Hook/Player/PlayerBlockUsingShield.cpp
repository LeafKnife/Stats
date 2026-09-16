#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorHurtResult.h>
#include <mc/world/actor/HurtParameters.h>
#include <mc/world/actor/player/Player.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    PlayerHurtShieldHook,
    HookPriority::Normal,
    Player,
    &Player::$_hurt,
    ::ActorHurtResult,
    ::ActorDamageSource const& source,
    float                      damage,
    ::HurtParameters const&    hurtParameters
) {
    auto& player = *thisFor<Player>();

    // _blockUsingShield was inlined into Player::_hurt. This is its original guard.
    auto const shieldBlockSucceeded = player.isDamageBlocked(source) && !player.getCurrentActiveShield().isNull();
    if (shieldBlockSucceeded) {
        handler::onPlayerBlockUsingShield(&player, damage);
    }

    return origin(source, damage, hurtParameters);
}

void hookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::hook();
}
void unhookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::unhook();
}
} // namespace stats::hook::player
