#include "mod/Events/BlockEventHandle.h"

#include "mc/world/level/block/CampfireBlock.h"
#include "ll/api/memory/Hook.h"

#include "mod/Hook/Hook.h"

namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    CampfireBlockUseHook,
    HookPriority::Normal,
    CampfireBlock,
    &CampfireBlock::$use,
    bool,
    ::Player&         player,
    ::BlockPos const& pos,
    uchar             face
) {
    auto res = origin(player, pos, face);
    // logger.info("CampfireBlockUseHook {} {} {}", player.getRealName(), pos, res);
    if (!res) return res;
    event::block::onCampfireBlockUsed(player);
    return res;
}

void hookCampfireBlockUse() { CampfireBlockUseHook::hook(); }

} // namespace stats::hook::block