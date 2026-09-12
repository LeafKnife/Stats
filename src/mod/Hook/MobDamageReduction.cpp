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

#include "mod/Stats/Stats.h"
#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::mob {
namespace {

float getResistanceDamage(float damageBeforeResistance, int amplifier) {
    auto const resistanceLevel = amplifier + 1;
    if (resistanceLevel <= 0) return 0.0f;

    auto const resistanceFactor = resistanceLevel >= 5 ? 0.0f : 1.0f - 0.2f * resistanceLevel;
    return std::abs(damageBeforeResistance * (1.0f - resistanceFactor));
}

void recordResistanceDamage(Mob& victim, ActorDamageSource const& source, float resistanceDamage) {
    if (resistanceDamage <= 0.0f) return;

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
    auto const afterEnchantReduction = origin(source, damage);
    auto const* resistance = this->getEffect(*MobEffect::DAMAGE_RESISTANCE());

    if (resistance && source.isReducedByResistanceEffect()) {
        auto const resistanceDamage = getResistanceDamage(afterEnchantReduction, resistance->mAmplifier);
        recordResistanceDamage(*thisFor<Mob>(), source, resistanceDamage);
    }

    // This is immediately before the inlined resistance calculation.
    if (this->isType(::ActorType::Player)) {
        // getLogger().info(
        //     "[DEBUG-enchant-reduction] victim={} tag='{}' damage_after_armor={:.3f} "
        //     "damage_before_resistance={:.3f} resistance_amplifier={} resistance_damage={:.3f} "
        //     "resistance_applies={}",
        //     this->getTypeName(),
        //     this->getNameTag(),
        //     damage,
        //     afterEnchantReduction,
        //     resistance ? resistance->mAmplifier : -1,
        //     resistance && source.isReducedByResistanceEffect()
        //         ? getResistanceDamage(afterEnchantReduction, resistance->mAmplifier)
        //         : 0.0f,
        //     source.isReducedByResistanceEffect()
        // );
    }

    return afterEnchantReduction;
}

void hookMobGetDamageAfterEnchantReduction() { MobGetDamageAfterEnchantReductionHook::hook(); }
void unhookMobGetDamageAfterEnchantReduction() { MobGetDamageAfterEnchantReductionHook::unhook(); }

} // namespace stats::hook::mob
