#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "lk/stats/event/PlayerEventHandle.h"

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
    
    event::player::onUsedItem(player, instance, itemUseMethod, consumeItem);
    return origin(instance, itemUseMethod, consumeItem);
}

void hookPlayerUseItem(){
    PlayerUseItemHook::hook();
}
//PlayerEat BlockPlace 也可以通过hook玩家使用物品
} // namespace stats::hook::player