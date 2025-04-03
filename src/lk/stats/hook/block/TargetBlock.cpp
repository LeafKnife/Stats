#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/level/block/TargetBlock.h>

#include "lk/stats/event/BlockEventHandle.h"


namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    TargetBlockOnProjectileHitHook,
    HookPriority::Normal,
    TargetBlock,
    &TargetBlock::$onProjectileHit,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor const&    projectile
) {
    event::block::onProjectileHitTargetBlock(projectile);
    return origin(region, pos, projectile);
}

void hookProjectileHitTargetBlock(){
    TargetBlockOnProjectileHitHook::hook();
}

} // namespace stats::hook::block