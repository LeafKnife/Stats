#include "lk/stats/event/BlockEventHandle.h"
#include "lk/stats/event/PlayerEventHandle.h"
#include "lk/stats/hook/Hook.h"
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

    Player&   player    = mUnk84d147.as<Player&>();
    BlockPos& blockPos  = mUnk74419a.as<BlockPos>();
    auto      slot      = slotNumber + this->_getContainerOffset();
    auto&     block     = event::block::getBlockByBlockPos(blockPos, player.getDimensionId());
    auto      blockType = block.getTypeName();
    event::player::onChangeContainerWith(player, blockType, slot, oldItem, newItem);
    origin(slotNumber, oldItem, newItem);
}

void hookLevelContainerChanged() { LevelContainerChangeHook::hook(); }
} // namespace stats::hook::container