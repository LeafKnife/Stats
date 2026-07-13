#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemStackBase.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::item {
LL_TYPE_INSTANCE_HOOK(
    ItemStackBaseHurtAndBreak,
    HookPriority::Normal,
    ItemStackBase,
    &ItemStackBase::hurtAndBreak,
    bool,
    int      deltaDamage,
    ::Actor* owner
) {
    if (!owner||!owner->isType(::ActorType::Player)) return origin(deltaDamage, owner);
    ItemStackBase* item = this;
    auto           res  = origin(deltaDamage, owner);
    if (!res) return res;
    Player* player = owner->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return res;
    handler::onPlayerItemHurtAndBroken(player, item, deltaDamage);
    return res;
}

void hookItemStackBaseHurtAndBroken() { ItemStackBaseHurtAndBreak::hook(); }
void unhookItemStackBaseHurtAndBroken() { ItemStackBaseHurtAndBreak::unhook(); }
} // namespace stats::hook::item
