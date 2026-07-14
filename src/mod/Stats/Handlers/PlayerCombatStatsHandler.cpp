#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

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

void onPlayerTakenDamage(Player* player, float damage, float afterDamage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto  health              = player->getHealth();
    auto  absorptionAttribute = player->getAttribute(SharedAttributes::ABSORPTION());
    auto  absorption          = absorptionAttribute.mPtr ? absorptionAttribute.mPtr->mCurrentValue : 0.0f;
    float damageTaken     = afterDamage > 0 ? afterDamage : -afterDamage;
    float damageAbsorbed  = 0;
    if (absorption > 0) {
        damageAbsorbed  = damageTaken < absorption ? damageTaken : absorption;
        damageTaken    -= damageAbsorbed;
    }
    float resistanceDamage = damage - afterDamage;
    resistanceDamage       = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
    damageTaken            = damageTaken < health ? damageTaken : health;
    playerStats->addCustomStats(CustomType::damage_resisted, static_cast<int>(resistanceDamage * 10));
    playerStats->addCustomStats(CustomType::damage_absorbed, static_cast<int>(damageAbsorbed * 10));
    playerStats->addCustomStats(CustomType::damage_taken, static_cast<int>(damageTaken * 10));
}

void onPlayerDealtDamage(Mob* mob, Player* player, float damage, float afterDamage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto  health              = mob->getHealth();
    auto  absorptionAttribute = mob->getAttribute(SharedAttributes::ABSORPTION());
    auto  absorption          = absorptionAttribute.mPtr ? absorptionAttribute.mPtr->mCurrentValue : 0.0f;
    float damageTaken     = afterDamage > 0 ? afterDamage : -afterDamage;
    float damageAbsorbed  = 0;
    if (absorption > 0) {
        damageAbsorbed  = damageTaken < absorption ? damageTaken : absorption;
        damageTaken    -= damageAbsorbed;
    }
    float resistanceDamage = damage - afterDamage;
    resistanceDamage       = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
    damageTaken            = damageTaken < health ? damageTaken : health;
    playerStats->addCustomStats(CustomType::damage_dealt_resisted, static_cast<int>(resistanceDamage * 10));
    playerStats->addCustomStats(CustomType::damage_dealt_absorbed, static_cast<int>(damageAbsorbed * 10));
    playerStats->addCustomStats(CustomType::damage_dealt, static_cast<int>(damageTaken * 10));
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
    playerStats->addCustomStats(CustomType::damage_blocked_by_shield, static_cast<int>(damage * 10));
}

} // namespace stats::handler
