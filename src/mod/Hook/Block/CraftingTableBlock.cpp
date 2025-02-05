#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/level/block/CraftingTableBlock.h"

#include "mod/Events/BlockEventHandle.h"

namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    CraftingTableUseHook,
    HookPriority::Normal,
    CraftingTableBlock,
    &CraftingTableBlock::$use,
    bool,
    ::Player&         player,
    ::BlockPos const& pos,
    uchar             face
) {
    auto res = origin(player, pos, face);
    // logger.info("CraftingTableBlockUseHook {} {} {}", player.getRealName(), pos, res);
    if (!res) return res;
    event::block::onCraftingTableBlockUsed(player);
    return res;
}

void hookCraftingTableUse() { CraftingTableUseHook::hook(); }

} // namespace stats::hook::block