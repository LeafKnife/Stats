#include "mod/Events/PlayerEventHandle.h"

#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/item/ItemUseMethod.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"

#include "mod/Stats/Stats.h"

namespace stats {
namespace event {
namespace player {
namespace {
auto& playerStatsMap = getPlayerStatsMap();
}

void onJoin(Player& player) {
    if (player.isSimulatedPlayer()) return;
    //PlayerStats* playerStats = new PlayerStats(player);
    auto playerStats = std::make_shared<PlayerStats>(player);
    playerStatsMap.emplace(playerStats->getUuid(), playerStats);
}

void onLeft(Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::leave_game);
    playerStats->saveData();
    // delete playerStats;
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

void onTakenDamage(Player* player, float damage, float afterDamage) {
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;

    auto  heath          = player->getMutableAttribute(SharedAttributes::HEALTH())->getCurrentValue();
    auto  absorption     = player->getMutableAttribute(SharedAttributes::ABSORPTION())->getCurrentValue();
    float damage_taken   = afterDamage > 0 ? afterDamage : -afterDamage;
    float damage_absobed = 0;
    if (absorption > 0) {
        if (damage_taken >= absorption) {
            damage_taken   = damage_taken - absorption;
            damage_absobed = absorption;
        } else {
            damage_taken   = 0;
            damage_absobed = damage_taken;
        }
    }
    float resistanceDamage = damage - afterDamage;
    resistanceDamage       = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
    damage_taken           = damage_taken < heath ? damage_taken : heath;
    playerStats->addCustomStats(CustomType::damage_resisted, static_cast<int>(resistanceDamage * 10));
    playerStats->addCustomStats(CustomType::damage_absorbed, static_cast<int>(damage_absobed * 10));
    playerStats->addCustomStats(CustomType::damage_taken, static_cast<int>(damage_taken * 10));
}

void onDealtDamage(Mob* mob, Player* player, float damage, float afterDamage) {
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;

    auto  heath          = mob->getMutableAttribute(SharedAttributes::HEALTH())->getCurrentValue();
    auto  absorption     = mob->getMutableAttribute(SharedAttributes::ABSORPTION())->getCurrentValue();
    float damage_taken   = afterDamage > 0 ? afterDamage : -afterDamage;
    float damage_absobed = 0;
    if (absorption > 0) {
        if (damage_taken >= absorption) {
            damage_taken   = damage_taken - absorption;
            damage_absobed = absorption;
        } else {
            damage_taken   = 0;
            damage_absobed = damage_taken;
        }
    }
    float resistanceDamage = damage - afterDamage;
    resistanceDamage       = resistanceDamage > 0 ? resistanceDamage : -resistanceDamage;
    damage_taken           = damage_taken < heath ? damage_taken : heath;
    playerStats->addCustomStats(CustomType::damage_dealt_resisted, static_cast<int>(resistanceDamage * 10));
    playerStats->addCustomStats(CustomType::damage_dealt_absorbed, static_cast<int>(damage_absobed * 10));
    playerStats->addCustomStats(CustomType::damage_dealt, static_cast<int>(damage_taken * 10));
}

void onCraftedItem();

void onItemHurtAndBroken(Player* player, ItemStackBase* item, int deltaDamage) {
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    if (!item->isDamageableItem()) return;
    if (!item->isArmorItem()) playerStats->addStats(StatsDataType::used, item->getTypeName());
    auto maxDamage = item->getMaxDamage();
    auto damage    = item->getDamageValue();
    if (damage + deltaDamage > maxDamage) {
        playerStats->addStats(StatsDataType::broken, item->getTypeName());
    }
}

void onUsedItem(Player* player, ItemStackBase& instance, ItemUseMethod itemUseMethod, bool consumeItem) {
    // 奶桶 空桶 钓鱼竿 弓 工具/武器 书与笔 空地图监听不到需要单独处理
    // 玩家的统计数据在生物身上使用物品时不会增加
    //  - 无论是命名、驯服、喂养、繁殖、上鞍、拴住、剪毛、染色、挤奶还是收集炖菜
    //  - 当盔甲按使用键装备时，当皮革盔甲在炼药锅中清洗时，以及上面提到的例子。
    if (!consumeItem) return;
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    switch (itemUseMethod) {
    case ItemUseMethod::Eat:
    case ItemUseMethod::Consume:
    case ItemUseMethod::Throw:
    case ItemUseMethod::FillBottle:
    case ItemUseMethod::PourBucket:
        playerStats->addStats(StatsDataType::used, instance.getTypeName());
        break;
    case ItemUseMethod::Place:
        playerStats->addStats(StatsDataType::used, instance.getTypeName());
        if (instance.isMusicDiscItem()) playerStats->addCustomStats(CustomType::play_record);
        break;
    case ItemUseMethod::Interact:
        // 交互只记录骨粉，其他与实体交互均不记录
        if (instance.getTypeName() == "minecraft:bone_meal")
            playerStats->addStats(StatsDataType::used, instance.getTypeName());
        break;
    default:
        break;
    }
};


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
    if (player->isSimulatedPlayer()) return;
    auto uuid        = player->getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::damage_blocked_by_shield, static_cast<int>(damage * 10));
    return;
}

void onOpenTrading(Player* player) {
    if (player->isSimulatedPlayer()) return;
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

void onChangeContainerWith(
    Player&          player,
    std::string      blockType,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    if (player.isSimulatedPlayer()) return;
    auto uuid        = player.getUuid();
    auto playerStats = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    if (blockType == "minecraft:furnace" || blockType == "minecraft:lit_furnace" || blockType == "minecraft:smoker") {
        if (slot != 2) return;
        if (oldItem.isNull()) return;
        if (oldItem.mCount > newItem.mCount) {
            playerStats->addStats(StatsDataType::crafted, oldItem.getTypeName());
        }
    }
    return;
}
} // namespace player
} // namespace event
} // namespace stats