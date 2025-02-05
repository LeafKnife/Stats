#include "mod/Hook/Hook.h"

#include "mod/MyMod.h"

namespace stats::hook {
void hook() {
    player::hookPlayerAddEffect();
    player::hookPlayerBlockUsingShield();
    player::hookPlayerDropItem();
    player::hookPlayerStartSleep();
    player::hookServerPlayerOpenTrading();
    player::hookPlayerUseItem();

    mob::hookMobGetDamageAfterResistanceEffect();

    block::hookBlockInteractedWith();
    block::hookCakeRemoveSlice();
    block::hookCampfireBlockUse();
    block::hookCauldronBlockUseInventory();
    block::hookCauldronBlockClean();
    block::hookCraftingTableUse();
    block::hookFlowerPotBlockPlaceFlower();
    block::hookNoteBlockAttack();
    block::hookProjectileHitTargetBlock();

    //item::hookFishingRodUsed();
    item::hookItemStackBaseHurtAndBroken();

    container::hookLevelContainerChanged();
}

void unhook() {}

ll::io::Logger& getLogger() { return lk::MyMod::getInstance().getSelf().getLogger(); }
} // namespace stats::hook