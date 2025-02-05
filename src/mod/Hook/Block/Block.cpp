#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/events/BlockEventCoordinator.h"

#include "mod/Events/BlockEventHandle.h"

namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    BlockInteractedWithHook,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockInteractedWith,
    void,
    ::Player&         player,
    ::BlockPos const& blockPos
) {
    event::block::onBlockInteracted(blockPos, player);
    return origin(player, blockPos);
    // 营火~ 唱片机 工作台~ 监听不到
}

void hookBlockInteractedWith(){
    BlockInteractedWithHook::hook();
}

} // namespace stats::hook::block