#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "lk/stats/event/PlayerEventHandle.h"

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
    event::player::onBlockUsingShield(player, damage);
    return res;
}

void hookPlayerBlockUsingShield(){
    PlayerBlockUsingShieldHook::hook();
}
} // namespace stats::hook::player