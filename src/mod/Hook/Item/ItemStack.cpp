// #include "mod/Hook/Hook.h"

// #include "ll/api/memory/Hook.h"
// #include "mc/deps/ecs/WeakEntityRef.h"
// #include "mc/world/item/FishingRodItem.h"
// #include "mc/world/item/ItemStackBase.h"

// #include "mod/Events/PlayerEventHandle.h"

// namespace stats::hook::item {
// LL_TYPE_INSTANCE_HOOK(
//     PlayerEatHook2,
//     HookPriority::Normal,
//     ItemStack,
//     &ItemStack::useTimeDepleted,
//     ::ItemUseMethod,
//     Level*  level,
//     Player* player
// ) {
//     if (getTypeName() == "minecraft:milk_bucket") {}
//     return origin(level, player);
// }

// void hookItemStackBaseHurtAndBroken() { ItemStackBaseHurtAndBreak::hook(); }
// } // namespace stats::hook::item