#include "mc/world/effect/MobEffectIds.h"
#include "mod/Hook/Hook.h"

#include <cmath>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/effect/MobEffect.h>
#include <mc/world/effect/MobEffectInstance.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"
#include "mod/Stats/Stats.h"


namespace stats::hook::mob {
namespace {

float getResistanceDamage(float damageBeforeResistance, int amplifier) {
    if (!std::isfinite(damageBeforeResistance) || damageBeforeResistance <= 0.0f || amplifier < 0) return 0.0f;
    if (amplifier >= 4) return damageBeforeResistance;
    return damageBeforeResistance * 0.2f * static_cast<float>(amplifier + 1);
}

void recordResistanceDamage(Mob& victim, ActorDamageSource const& source, float resistanceDamage) {
    if (!std::isfinite(resistanceDamage) || resistanceDamage <= 0.0f) return;

    if (victim.isType(::ActorType::Player)) {
        if (auto* player = victim.getEntityContext().getWeakRef().tryUnwrap<Player>().as_ptr()) {
            handler::onPlayerResistedDamage(player, resistanceDamage);
        }
    }

    // Preserve the previous scope: only direct player attacks count as dealt damage.
    if (!source.isEntitySource() || source.isChildEntitySource()) return;

    auto level = ll::service::getLevel();
    if (!level) return;

    auto* attacker = level->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (!attacker || !attacker->isType(::ActorType::Player)) return;

    if (auto* player = attacker->getEntityContext().getWeakRef().tryUnwrap<Player>().as_ptr()) {
        handler::onPlayerDealtResistedDamage(player, resistanceDamage);
    }
}

} // namespace

LL_TYPE_INSTANCE_HOOK(
    MobGetDamageAfterEnchantReductionHook,
    HookPriority::Normal,
    Mob,
    &Mob::$getDamageAfterEnchantReduction,
    float,
    ::ActorDamageSource const& source,
    float                      damage
) {
    auto const  afterEnchantReduction = origin(source, damage);
    auto const* resistance            = this->getEffect((uint)MobEffectIds::DamageResistance);

    if (resistance && source.isReducedByResistanceEffect()) {
        auto const resistanceDamage = getResistanceDamage(afterEnchantReduction, resistance->mAmplifier);
        recordResistanceDamage(*thisFor<Mob>(), source, resistanceDamage);
    }

    return afterEnchantReduction;
}

void hookMobGetDamageAfterEnchantReduction() { MobGetDamageAfterEnchantReductionHook::hook(); }
void unhookMobGetDamageAfterEnchantReduction() { MobGetDamageAfterEnchantReductionHook::unhook(); }

} // namespace stats::hook::mob
