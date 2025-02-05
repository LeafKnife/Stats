#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/world/item/FishingRodItem.h"
#include "mc/world/item/ItemStackBase.h"

#include "mod/Events/PlayerEventHandle.h"

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
    if (!owner->hasType(::ActorType::Player)) return origin(deltaDamage, owner);
    ItemStackBase* item = this;
    auto           res  = origin(deltaDamage, owner);
    // logger.info(
    //     "ItemStackBaseHurtAndBreak {} {} {} {}",
    //     this->getTypeName(),
    //     deltaDamage,
    //     owner->getTypeName(),
    //     res
    // );
    if (!res) return res;
    Player* player = owner->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return res;
    event::player::onItemHurtAndBroken(player, item, deltaDamage);
    return res;
}

void hookItemStackBaseHurtAndBroken() { ItemStackBaseHurtAndBreak::hook(); }
} // namespace stats::hook::item