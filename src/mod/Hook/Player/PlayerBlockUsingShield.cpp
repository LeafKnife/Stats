#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"

#include "mod/Events/PlayerEventHandle.h"

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
    // logger.info("PlayerBlockUsingShieldHook {} {} {}", player->getRealName(), damage, res);
    if (!res) return res;
    event::player::onBlockUsingShield(player, damage);
    return res;
}

void hookPlayerBlockUsingShield(){
    PlayerBlockUsingShieldHook::hook();
}
} // namespace stats::hook::player