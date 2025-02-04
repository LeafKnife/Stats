#pragma once

namespace stats {
namespace hook {
void hook();
void unhook();

namespace player {
void hookPlayerAddEffect();
void hookPlayerBlockUsingShield();
void hookPlayerDropItem();
void hookPlayerStartSleep();
void hookServerPlayerOpenTrading();
} // namespace player

namespace block {
void hookBlockInteractedWith();
void hookCakeRemoveSlice();
void hookCampfireBlockUse();
void hookCauldronBlockUseInventory();
void hookCauldronBlockClean();
void hookCraftingTableUse();
void hookFlowerPotBlockPlaceFlower();
void hookNoteBlockAttack();
void hookProjectileHitTargetBlock();
} // namespace block
} // namespace hook
} // namespace stats