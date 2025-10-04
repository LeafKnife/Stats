#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/level/Level.h>

#include "mod/Events/PlayerEventHandle.h"

namespace stats::hook::mob {

void onMobHurt(Mob* mob, ActorDamageSource const& source, float damage, float afterDamage) {
    if (mob->isType(::ActorType::Player)) {
        Player* player = mob->getEntityContext().getWeakRef().tryUnwrap<Player>();
        if (!player) return;
        event::player::onTakenDamage(player, damage, afterDamage);
    }
    if (source.isEntitySource()) {
        Actor* damageSource = nullptr;
        // 玩家造成的伤害总量，单位为1（♥）的1⁄10，只统计近战攻击造成的伤害。
        if (source.isChildEntitySource()) return;
        damageSource = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
        if (damageSource->isType(::ActorType::Player)) {
            Player* player = damageSource->getEntityContext().getWeakRef().tryUnwrap<Player>();
            if (!player) return;
            event::player::onDealtDamage(mob, player, damage, afterDamage);
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    MobGetDamageAfterResistanceEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ::ActorDamageSource const& source,
    float                      damage
) {
    auto afterDamage = origin(source, damage);
    Mob* mob         = this;
    onMobHurt(mob, source, damage, afterDamage);
    return afterDamage;
}

void hookMobGetDamageAfterResistanceEffect() { MobGetDamageAfterResistanceEffectHook::hook(); }
} // namespace stats::hook::mob
