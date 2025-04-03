#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/level/block/CakeBlock.h>

#include "lk/stats/event/BlockEventHandle.h"

namespace stats::hook::block {
LL_TYPE_STATIC_HOOK(
    CakeRemoveSliceHook,
    HookPriority::Normal,
    CakeBlock,
    &CakeBlock::removeCakeSlice,
    void,
    ::Player&         player,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Block const*    cakeBlock
) {
    event::block::onCakeBlockRemovedSlice(player);
    return origin(player, region, pos, cakeBlock);
}

void hookCakeRemoveSlice() { CakeRemoveSliceHook::hook(); }

} // namespace stats::hook::block