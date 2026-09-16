#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <ll/api/service/Bedrock.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/AttributeInstanceConstRef.h>
#include <mc/world/attribute/AttributeModificationContext.h>
#include <mc/world/attribute/MutableAttributeWithContext.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/effect/MobEffectInstance.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Stats.h"

namespace stats::handler {
namespace {

constexpr double MaxUint64Exclusive = 18446744073709551616.0;

float positiveFinite(float value) {
    return std::isfinite(value) && value > 0.0f ? value : 0.0f;
}

uint64_t toDamageStatValue(float damage) {
    auto const scaled = static_cast<double>(positiveFinite(damage)) * 10.0;
    if (!std::isfinite(scaled) || scaled >= MaxUint64Exclusive) return 0;
    return static_cast<uint64_t>(scaled);
}

void addDamageStat(PlayerStats& stats, CustomType type, float damage) {
    stats.addCustomStats(type, toDamageStatValue(damage));
}

} // namespace

void onPlayerDied(Player& player, ActorDamageSource const& source) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::deaths);
    if (!source.isEntitySource()) return;

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (!actor) return;
    if (source.isChildEntitySource()) actor = actor->getOwner();
    if (!actor) return;
    playerStats->addStats(StatsType::killed_by, actor->getTypeName());
}

void onPlayerKillMob(Player& player, Mob& mob) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    if (mob.isType(ActorType::Player)) {
        playerStats->addCustomStats(CustomType::player_kills);
    } else {
        playerStats->addCustomStats(CustomType::mob_kills);
    }
    playerStats->addStats(StatsType::killed, mob.getTypeName());
}

void onPlayerTakenDamage(Player* player, float finalDamage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto  health              = positiveFinite(player->getHealth());
    auto  absorptionAttribute = player->getAttribute(SharedAttributes::ABSORPTION());
    auto  absorption          = absorptionAttribute.mPtr ? positiveFinite(absorptionAttribute.mPtr->mCurrentValue) : 0.0f;
    float damageTaken     = positiveFinite(finalDamage);
    if (damageTaken == 0.0f) return;
    float damageAbsorbed  = 0;
    if (absorption > 0) {
        damageAbsorbed  = std::min(damageTaken, absorption);
        damageTaken    -= damageAbsorbed;
    }
    damageTaken = std::min(damageTaken, health);
    addDamageStat(*playerStats, CustomType::damage_absorbed, damageAbsorbed);
    addDamageStat(*playerStats, CustomType::damage_taken, damageTaken);
}

void onPlayerDealtDamage(Mob* mob, Player* player, float finalDamage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto  health              = positiveFinite(mob->getHealth());
    auto  absorptionAttribute = mob->getAttribute(SharedAttributes::ABSORPTION());
    auto  absorption          = absorptionAttribute.mPtr ? positiveFinite(absorptionAttribute.mPtr->mCurrentValue) : 0.0f;
    float damageTaken     = positiveFinite(finalDamage);
    if (damageTaken == 0.0f) return;
    float damageAbsorbed  = 0;
    if (absorption > 0) {
        damageAbsorbed  = std::min(damageTaken, absorption);
        damageTaken    -= damageAbsorbed;
    }
    damageTaken = std::min(damageTaken, health);
    addDamageStat(*playerStats, CustomType::damage_dealt_absorbed, damageAbsorbed);
    addDamageStat(*playerStats, CustomType::damage_dealt, damageTaken);
}

void onPlayerResistedDamage(Player* player, float resistanceDamage) {
    auto* playerStats = findPlayerStats(player->getUuid());
    if (!playerStats) return;

    addDamageStat(*playerStats, CustomType::damage_resisted, resistanceDamage);
}

void onPlayerDealtResistedDamage(Player* player, float resistanceDamage) {
    auto* playerStats = findPlayerStats(player->getUuid());
    if (!playerStats) return;

    addDamageStat(*playerStats, CustomType::damage_dealt_resisted, resistanceDamage);
}

void onPlayerEffectAdded(Player* player, MobEffectInstance const& effect) {
    auto  effectId      = effect.mId;
    auto  durationValue = effect.mDuration->mValue;
    auto  uuid          = player->getUuid();
    auto* playerStats   = findPlayerStats(uuid);
    if (!playerStats) return;
    if (effectId == 29 && durationValue == 40 * 60 * 20) {
        playerStats->addCustomStats(CustomType::raid_win);
    } else if (effectId == 36 && durationValue == 30 * 20) {
        playerStats->addCustomStats(CustomType::raid_trigger);
    }
}

void onPlayerBlockUsingShield(Player* player, float damage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    addDamageStat(*playerStats, CustomType::damage_blocked_by_shield, damage);
}

} // namespace stats::handler
