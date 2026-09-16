#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "mod/Events/BlockEventHandle.h"

#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/FlowerPotBlock.h>
#include <mc/world/level/block/actor/FlowerPotBlockActor.h>
#include <mc/world/level/block/block_events/BlockPlayerInteractEvent.h>

namespace stats::hook::block {
namespace {

Block const* getPottedPlant(BlockEvents::BlockPlayerInteractEvent const& eventData) {
    auto const* blockActor = eventData.getBlockSource().getBlockEntity(eventData.mPos);
    if (!blockActor) return nullptr;

    return static_cast<FlowerPotBlockActor const*>(blockActor)->mPlant;
}

} // namespace

LL_TYPE_INSTANCE_HOOK(
    FlowerPotBlockTryPlaceFlowerHook,
    HookPriority::Normal,
    FlowerPotBlock,
    &FlowerPotBlock::use,
    void,
    ::BlockEvents::BlockPlayerInteractEvent& eventData
) {
    auto const* plantBefore = getPottedPlant(eventData);

    origin(eventData);

    auto const* plantAfter = getPottedPlant(eventData);
    if (!eventData.mSuccessful->value_or(false) || plantBefore || !plantAfter) return;

    event::block::onFlowerPotBlockPlaceFlower(eventData.mPlayer);
    return;
}

void hookFlowerPotBlockPlaceFlower() {
    FlowerPotBlockTryPlaceFlowerHook::hook();
}
void unhookFlowerPotBlockPlaceFlower() {
    FlowerPotBlockTryPlaceFlowerHook::unhook();
}

} // namespace stats::hook::block
