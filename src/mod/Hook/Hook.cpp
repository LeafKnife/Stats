#include "mod/Hook/Hook.h"

namespace stats::hook {
void hook() {
    player::hookPlayerAddEffect();
    player::hookPlayerBlockUsingShield();
    player::hookPlayerDropItem();
    player::hookPlayerStartSleep();
    player::hookServerPlayerOpenTrading();
    player::hookPlayerUseItem();
    player::hookPlayerInteractActor();
    player::hookPlayerBreedAnimal();
    player::hookPlayerUseBucketItemOnFish();
    player::hookPlayerRiding();
    player::hookPlayerAuthInput();

    mob::hookMobGetDamageAfterResistanceEffect();

    block::hookBlockUse();
    block::hookBlockOnFallOn();
    block::hookCakeRemoveSlice();
    block::hookCauldronBlockUseInventory();
    block::hookCauldronBlockClean();
    block::hookFlowerPotBlockPlaceFlower();
    block::hookNoteBlockAttack();
    block::hookProjectileHitTargetBlock();

    // item::hookFishingRodUsed();
    item::hookItemStackBaseHurtAndBroken();

    container::hookLevelContainerChanged();

    // test::hookTest();
}

void unhook() {}
} // namespace stats::hook