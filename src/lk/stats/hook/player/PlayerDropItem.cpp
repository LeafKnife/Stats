#include "lk/stats/hook/Hook.h"

#include <ll/api/memory/Hook.h>

#include "lk/stats/event/PlayerEventHandle.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook,
    HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& item,
    bool             randomly
) {
    auto res = origin(item, randomly);
    if (!res || randomly) return res;
    Player* player = this;
    event::player::onDropItem(player, item);
    return res;
}

void hookPlayerDropItem() { PlayerDropItemHook::hook(); }
} // namespace stats::hook::player