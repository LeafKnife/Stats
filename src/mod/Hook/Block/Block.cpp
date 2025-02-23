#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/CachedComponentData.h>
#include <mc/world/level/block/components/BlockComponentDirectData.h>
#include <mc/world/level/block/components/BlockComponentStorage.h>

#include "mod/Events/BlockEventHandle.h"

namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    BlockUseHook,
    HookPriority::Normal,
    ::Block,
    &Block::use,
    bool,
    ::Player&               player,
    ::BlockPos const&       pos,
    uchar                   face,
    ::std::optional<::Vec3> hit
) {
    auto r = origin(player, pos, face, hit);
    if (!r) return r;
    // getLogger().info("Hook {} {} {}", player.getRealName(), getTypeName(), r);
    event::block::onBlockUsed(pos, player);
    return r;
}

LL_TYPE_INSTANCE_HOOK(
    BlockOnFallOnHook,
    HookPriority::Normal,
    Block,
    &Block::onFallOn,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor&          entity,
    float             fallDistance
) {
    
    if (fallDistance < 1.0f) return origin(region, pos, entity, fallDistance);
    // getLogger().info(
    //     "BlockOnFallOnHook {} {} {}",
    //     fallDistance,
    //     entity.getTypeName(),
    //     getTypeName()
    // );
    event::block::onFallOn(entity, fallDistance);
    return origin(region, pos, entity, fallDistance);
}

void hookBlockUse() { BlockUseHook::hook(); }
void hookBlockOnFallOn() { BlockOnFallOnHook::hook(); }
} // namespace stats::hook::block