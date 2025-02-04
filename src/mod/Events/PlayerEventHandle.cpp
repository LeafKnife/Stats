#include "mod/Events/PlayerEventHandle.h"

#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/level/Level.h"
#include "mod/Stats/Stats.h"

namespace stats {
namespace event {
namespace player {
namespace {
auto& playerStatsMap = getPlayerStatsMap();
}

void onJoin(Player& player) {
    if (player.isSimulatedPlayer()) return;
    PlayerStats* playerStats = new PlayerStats(player);
    playerStatsMap.emplace(playerStats->getUuid(), playerStats);
}

void onLeft(Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::leave_game);
    playerStats->saveData();
    delete playerStats;
    playerStatsMap.erase(uuid);
}

void onTakeItem(Player& player, ItemStack& item) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addStats(StatsDataType::picked_up, item.getTypeName(), item.mCount);
}

void onDropItem(Player* player, ItemStack const& item) {
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::drop);
    playerStats->addStats(StatsDataType::dropped, item.getTypeName(), item.mCount);
}

void onDied(Player& player, ActorDamageSource const& source) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::deaths);
    // resetCustomSinceTime TODO
    if (!source.isEntitySource()) return;
    Actor* actor = nullptr;
    actor        = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (!actor) return;
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    playerStats->addStats(StatsDataType::killed_by, actor->getTypeName());
}

void onKillMob(Player& player, Mob& mob) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    if (mob.hasType(::ActorType::Player)) {
        playerStats->addCustomStats(CustomType::player_kills);
    } else {
        playerStats->addCustomStats(CustomType::mob_kills);
    }
    playerStats->addStats(StatsDataType::killed, mob.getTypeName());
}

// void playerHurt(Mob* mob, float damage, float afterDamage) {
//     Player* player = mob->getEntityContext().getWeakRef().tryUnwrap<Player>();
//     if (!player) return;
//     auto uuid        = player->getUuid();
//     auto playerStats = Stats::event::getPlayerStatsMap().find(uuid)->second;
//     if (!playerStats) return;
//     auto resistanceDamage = damage - afterDamage;
//     auto heath            = player->getMutableAttribute(SharedAttributes::HEALTH())->getCurrentValue();
//     auto absorption       = player->getMutableAttribute(SharedAttributes::ABSORPTION())->getCurrentValue();
//     resistanceDamage      = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
//     playerStats->addCustomStats(CustomType::damage_resisted, static_cast<int>(resistanceDamage * 10));
//     float damage_taken = afterDamage > 0 ? afterDamage : -afterDamage;
//     if (absorption > 0) {
//         float damage_absobed = damage_taken;
//         if (damage_taken >= absorption) {
//             damage_taken   = damage_taken - absorption;
//             damage_absobed = absorption;
//         } else {
//             damage_taken = 0;
//         }
//         playerStats->addCustomStats(CustomType::damage_absorbed, static_cast<int>(damage_absobed * 10));
//     }
//     damage_taken = damage_taken < heath ? damage_taken : heath;
//     playerStats->addCustomStats(CustomType::damage_taken, static_cast<int>(damage_taken * 10));
// }

// void mobHurtByPlayer(Mob* mob, Actor* damageSource, float damage, float afterDamage) {
//     Player* player = damageSource->getEntityContext().getWeakRef().tryUnwrap<Player>();
//     if (!player) return;
//     auto uuid        = player->getUuid();
//     auto playerStats = Stats::event::getPlayerStatsMap().find(uuid)->second;
//     if (!playerStats) return;
//     auto resistanceDamage = damage - afterDamage;
//     auto heath            = mob->getMutableAttribute(SharedAttributes::HEALTH())->getCurrentValue();
//     auto absorption       = mob->getMutableAttribute(SharedAttributes::ABSORPTION())->getCurrentValue();
//     resistanceDamage      = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
//     playerStats->addCustomStats(CustomType::damage_dealt_resisted, static_cast<int>(resistanceDamage * 10));
//     float damage_taken = afterDamage > 0 ? afterDamage : -afterDamage;
//     if (absorption > 0) {
//         float damage_absobed = damage_taken;
//         if (damage_taken >= absorption) {
//             damage_taken   = damage_taken - absorption;
//             damage_absobed = absorption;
//         } else {
//             damage_taken = 0;
//         }
//         playerStats->addCustomStats(CustomType::damage_dealt_absorbed, static_cast<int>(damage_absobed * 10));
//     }
//     damage_taken = damage_taken < heath ? damage_taken : heath;
//     playerStats->addCustomStats(CustomType::damage_dealt, static_cast<int>(damage_taken * 10));
// }

void onCraftedItem();
void onBrokenItem();
void onUsedItem();

void onEffectAdded(Player* player, MobEffectInstance const& effect) {
    auto effectId      = effect.getId();
    auto durationValue = effect.getDuration().mValue;
    auto uuid          = player->getUuid();
    auto playerStats   = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    if (effectId == 29 && durationValue == 40 * 60 * 20) {
        playerStats->addCustomStats(CustomType::raid_win);
    } else if (effectId == 36 && durationValue == 30 * 20) {
        playerStats->addCustomStats(CustomType::raid_trigger);
    }
}

void onStartSleep(Player* player) {
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::sleep_in_bed);
}

void onBlockUsingShield(Player* player, float damage) {
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::damage_blocked_by_shield, static_cast<int>(damage * 10));
    return;
}

void onOpenTrading(Player* player) {
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::talked_to_villager);
}

void onJump(Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::jump);
}

void onMove();
} // namespace player
} // namespace event
} // namespace stats