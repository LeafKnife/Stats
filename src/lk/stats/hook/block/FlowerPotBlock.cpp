#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "lk/stats/event/BlockEventHandle.h"

#include <mc/world/level/block/FlowerPotBlock.h>

namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    FlowerPotBlockTryPlaceFlowerHook,
    HookPriority::Normal,
    FlowerPotBlock,
    &FlowerPotBlock::_tryPlaceFlower,
    bool,
    ::Player&         player,
    ::BlockPos const& blockPos
) {
    auto res = origin(player, blockPos);
    if (!res) return res;
    event::block::onFlowerPotBlockPlaceFlower(player);
    return res;
}

void hookFlowerPotBlockPlaceFlower(){
    FlowerPotBlockTryPlaceFlowerHook::hook();
}

} // namespace stats::hook::block