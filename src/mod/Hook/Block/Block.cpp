#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/block/Block.h>

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
    //getLogger().info("Hook {} {} {}", player.getRealName(), getTypeName(), r);
    event::block::onBlockUsed(pos, player);
    return r;
}

void hookBlockUse() { BlockUseHook::hook(); }

} // namespace stats::hook::block