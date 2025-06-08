#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <mc/world/actor/player/Player.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/world/SimpleSparseContainer.h>
#include <mc/world/containers/FullContainerName.h>
#include <mc/world/inventory/network/ContainerScreenContext.h>
#include <mc/world/inventory/network/ItemStackNetResult.h>
#include <mc/world/inventory/network/ItemStackRequestActionHandler.h>
#include <mc/world/inventory/network/ItemStackRequestActionTransferBase.h>
#include <mc/world/inventory/network/ItemStackRequestSlotInfo.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/actor/BlockActor.h>

#include "mod/Events/PlayerEventHandle.h"

namespace stats::hook::inventory {

LL_TYPE_INSTANCE_HOOK(
    trasferHandlerHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::_handleTransfer,
    ::ItemStackNetResult,
    ::ItemStackRequestActionTransferBase const& requestAction,
    bool                                        isSwap,
    bool                                        isSrcHintSlot,
    bool                                        isDstHintSlot
) {
    auto& src = ll::memory::dAccess<ItemStackRequestSlotInfo>(&requestAction.mSrc, 4);
    // if (r != ItemStackNetResult::Success) return origin(requestAction, isSwap, isSrcHintSlot, isDstHintSlot);
    if (src.mFullContainerName.mName != ContainerEnumName::CreatedOutputContainer)
        return origin(requestAction, isSwap, isSrcHintSlot, isDstHintSlot);
    ::std::shared_ptr<::SimpleSparseContainer> srcContainer = _getOrInitSparseContainer(src.mFullContainerName);
    if (!srcContainer) return origin(requestAction, isSwap, isSrcHintSlot, isDstHintSlot);
    auto const& srcItem    = srcContainer->getItem(src.mSlot);
    auto&       screenCtx  = getScreenContext();
    auto        screenType = screenCtx.mScreenContainerType;
    auto        itemType   = srcItem.getTypeName();
    auto r = origin(requestAction, isSwap, isSrcHintSlot, isDstHintSlot);
    if (r != ItemStackNetResult::Success) return r;
    event::player::onCraftedItem(mPlayer.getUuid(), itemType, requestAction.mAmount, screenType);
    return r;
}
void hookItemStackRequestActionHandlerTransfer() { trasferHandlerHook::hook(); };
} // namespace stats::hook::inventory