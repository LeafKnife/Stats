#include "mod/Events/BlockEventHandle.h"
#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/level/block/CauldronBlock.h>


namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    CauldronBlockUseInventoryHook,
    HookPriority::Normal,
    CauldronBlock,
    &CauldronBlock::_useInventory,
    void,
    class Player&    player,
    class ItemStack& current,
    class ItemStack& replaceWith,
    int              useCount
) {
    // logger.info(
    //     "CauldronBlockUseInventoryHook {} {} {} {}",
    //     player.getRealName(),
    //     current.getTypeName(),
    //     replaceWith.getTypeName(),
    //     useCount
    // );
    event::block::onCauldronBlockUseInventory(player, current, replaceWith, useCount);
    return origin(player, current, replaceWith, useCount);
}

LL_TYPE_INSTANCE_HOOK(
    CauldronBlockCleanHook,
    HookPriority::Normal,
    CauldronBlock,
    &CauldronBlock::_sendCauldronUsedEventToClient,
    void,
    class Player const&                          player,
    short                                        itemId,
    ::MinecraftEventing::POIBlockInteractionType interactionType
) {
    event::block::onCauldronBlockClean(player, itemId, interactionType);
    return origin(player, itemId, interactionType);
}

void hookCauldronBlockUseInventory() { CauldronBlockUseInventoryHook::hook(); }

void hookCauldronBlockClean() { CauldronBlockCleanHook::hook(); }
} // namespace stats::hook::block