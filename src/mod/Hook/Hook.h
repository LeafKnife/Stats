#pragma once

namespace stats {
namespace hook {
void hook();
void unhook();

namespace player {
void hookPlayerAddEffect();
void unhookPlayerAddEffect();
void hookPlayerBlockUsingShield();
void unhookPlayerBlockUsingShield();
void hookPlayerDropItem();
void unhookPlayerDropItem();
void hookPlayerStartSleep();
void unhookPlayerStartSleep();
void hookServerPlayerOpenTrading();
void unhookServerPlayerOpenTrading();
void hookPlayerUseItem();
void unhookPlayerUseItem();
void hookPlayerInteractActor();
void unhookPlayerInteractActor();
void hookPlayerBreedAnimal();
void unhookPlayerBreedAnimal();
void hookPlayerUseBucketItemOnFish();
void unhookPlayerUseBucketItemOnFish();
void hookPlayerRiding();
void unhookPlayerRiding();
void hookPlayerAuthInput();
void unhookPlayerAuthInput();
} // namespace player

namespace mob {
void hookMobGetDamageAfterEnchantReduction();
void unhookMobGetDamageAfterEnchantReduction();
} // namespace mob

namespace block {
void hookBlockUse();
void unhookBlockUse();
void hookBlockOnFallOn();
void unhookBlockOnFallOn();
void hookCakeRemoveSlice();
void unhookCakeRemoveSlice();
void hookCauldronBlockUseInventory();
void unhookCauldronBlockUseInventory();
void hookCauldronBlockClean();
void unhookCauldronBlockClean();
void hookFlowerPotBlockPlaceFlower();
void unhookFlowerPotBlockPlaceFlower();
void hookNoteBlockAttack();
void unhookNoteBlockAttack();
void hookProjectileHitTargetBlock();
void unhookProjectileHitTargetBlock();
} // namespace block

namespace item {
// void hookFishingRodUsed();
void hookItemStackBaseHurtAndBroken();
void unhookItemStackBaseHurtAndBroken();
} // namespace item

namespace container {
void hookLevelContainerChanged();
void unhookLevelContainerChanged();
}

namespace inventory {
void hookItemStackRequestActionHandlerTransfer();
void unhookItemStackRequestActionHandlerTransfer();
}

} // namespace hook
} // namespace stats
