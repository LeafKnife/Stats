#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/player/Player.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    PlayerBlockUsingShieldHook,
    HookPriority::Normal,
    Player,
    &Player::_blockUsingShield,
    bool,
    ::ActorDamageSource const& source,
    float                      damage
) {
    auto    res    = origin(source, damage);
    Player* player = this;
    if (!res) return res;
    handler::onPlayerBlockUsingShield(player, damage);
    return res;
}

void hookPlayerBlockUsingShield(){
    PlayerBlockUsingShieldHook::hook();
}
void unhookPlayerBlockUsingShield() { PlayerBlockUsingShieldHook::unhook(); }
} // namespace stats::hook::player
