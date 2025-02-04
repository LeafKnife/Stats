#include "mod/Hook/Hook.h"

namespace stats::hook {
void hook() {
    player::hookPlayerAddEffect();
    player::hookPlayerBlockUsingShield();
    player::hookPlayerDropItem();
    player::hookPlayerStartSleep();
    player::hookServerPlayerOpenTrading();

    block::hookBlockInteractedWith();
    block::hookCakeRemoveSlice();
    block::hookCampfireBlockUse();
    block::hookCauldronBlockUseInventory();
    block::hookCauldronBlockClean();
    block::hookCraftingTableUse();
    block::hookFlowerPotBlockPlaceFlower();
    block::hookNoteBlockAttack();
    block::hookProjectileHitTargetBlock();
}

void unhook() {}
} // namespace stats::hook