#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemStack.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

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
    handler::onPlayerDropItem(player, item);
    return res;
}

void hookPlayerDropItem() { PlayerDropItemHook::hook(); }
void unhookPlayerDropItem() { PlayerDropItemHook::unhook(); }
} // namespace stats::hook::player
