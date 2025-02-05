// #include "mod/Hook/Hook.h"

// #include "ll/api/memory/Hook.h"
// #include "mc/world/item/FishingRodItem.h"
// #include "mc/world/item/ItemStack.h"

// #include "mod/Events/PlayerEventHandle.h"

// namespace stats::hook::item {
// LL_TYPE_INSTANCE_HOOK(
//     FishingRodUseHook,
//     HookPriority::Normal,
//     FishingRodItem,
//     &FishingRodItem::$use,
//     ::ItemStack&,
//     ::ItemStack& item,
//     ::Player&    player
// ) {
//     auto& res = origin(item, player);
//     getLogger().info("hook {} {} {}", item.getTypeName(), player.getRealName(), res.getTypeName());
//     return res;
// }

// void hookFishingRodUsed(){
//     FishingRodUseHook::hook();
// }
// } // namespace stats::hook::item