#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "mod/Events/PlayerEventHandle.h"

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
    
    // getLogger().info(
    //     "PlayerUseItemHook {} {} {} {}",
    //     player->getRealName(),
    //     instance.getTypeName(),
    //     static_cast<int>(itemUseMethod),
    //     consumeItem
    // );
    event::player::onUsedItem(player, instance, itemUseMethod, consumeItem);
    return origin(instance, itemUseMethod, consumeItem);
}

void hookPlayerUseItem(){
    PlayerUseItemHook::hook();
}
//PlayerEat BlockPlace 也可以通过hook玩家使用物品
} // namespace stats::hook::player