#include "mod/Events/BlockEventHandle.h"
#include "mod/Events/PlayerEventHandle.h"
#include "mod/Hook/Hook.h"
#include <ll/api/memory/Hook.h>
#include <mc/world/containers/models/LevelContainerModel.h>
#include <mc/world/item/ItemStack.h>


namespace stats::hook::container {
LL_TYPE_INSTANCE_HOOK(
    LevelContainerChangeHook,
    HookPriority::Normal,
    LevelContainerModel,
    &LevelContainerModel::$_onItemChanged,
    void,
    int              slotNumber,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    if (*reinterpret_cast<void***>(this) != LevelContainerModel::$vftable())
        return origin(slotNumber, oldItem, newItem);

    Player&   player    = mPlayer;
    BlockPos& blockPos  = mBlockPos;
    auto      slot      = slotNumber + this->_getContainerOffset();
    auto&     block     = event::block::getBlockByBlockPos(blockPos, player.getDimensionId());
    auto      blockType = block.getTypeName();
    event::player::onChangeContainerWith(player, blockType, slot, oldItem, newItem);
    origin(slotNumber, oldItem, newItem);
}

void hookLevelContainerChanged() { LevelContainerChangeHook::hook(); }
} // namespace stats::hook::container