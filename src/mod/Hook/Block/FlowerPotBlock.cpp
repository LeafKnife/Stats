#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "mod/Events/BlockEventHandle.h"

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
    // logger.info("FlowerPotBlockTryPlaceFlowerHook {} {} {}", player.getRealName(), blockPos, res);
    if (!res) return res;
    event::block::onFlowerPotBlockPlaceFlower(player);
    return res;
}

void hookFlowerPotBlockPlaceFlower(){
    FlowerPotBlockTryPlaceFlowerHook::hook();
}

} // namespace stats::hook::block