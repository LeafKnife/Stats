#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/player/BedSleepingResult.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockPos.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

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
    if (res == BedSleepingResult::Ok) handler::onPlayerStartSleep(player);
    return res;
}
void hookPlayerStartSleep() { PlayerStartSleepHook::hook(); }
void unhookPlayerStartSleep() { PlayerStartSleepHook::unhook(); }
} // namespace stats::hook::player
