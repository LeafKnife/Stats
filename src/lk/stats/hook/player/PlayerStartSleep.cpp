#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "lk/stats/event/PlayerEventHandle.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    PlayerStartSleepHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& pos
) {
    auto    res    = origin(pos);
    Player* player = this;
    if (res == BedSleepingResult::Ok) event::player::onStartSleep(player);
    return res;
}
void hookPlayerStartSleep() { PlayerStartSleepHook::hook(); }
} // namespace stats::hook::player