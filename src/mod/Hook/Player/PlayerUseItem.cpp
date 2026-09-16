#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/item/ItemUseMethod.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {

LL_TYPE_INSTANCE_HOOK(
    PlayerUseItemHook,
    HookPriority::Normal,
    Player,
    &Player::$useItem,
    void,
    ::ItemStackBase& instance,
    ::ItemUseMethod  itemUseMethod,
    bool             consumeItem
) {
    Player* player         = this;
    
    handler::onPlayerUsedItem(player, instance, itemUseMethod, consumeItem);
    return origin(instance, itemUseMethod, consumeItem);
}

void hookPlayerUseItem(){
    PlayerUseItemHook::hook();
}
void unhookPlayerUseItem() { PlayerUseItemHook::unhook(); }
//PlayerEat BlockPlace 也可以通过hook玩家使用物品
} // namespace stats::hook::player
