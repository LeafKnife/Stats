#include "mod/Events/PlayerEventHandle.h"

#include <ll/api/service/Bedrock.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/AttributeModificationContext.h>
#include <mc/world/attribute/MutableAttributeWithContext.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/item/ItemUseMethod.h>
#include <mc/world/item/components/ComponentItem.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>

#include "mod/Stats/Stats.h"

namespace stats {
namespace event {
namespace player {
namespace {
auto& playerStatsMap = getPlayerStatsMap();
}

void onJoin(Player& player) {
    if (player.isSimulatedPlayer()) return;
    // PlayerStats* playerStats = new PlayerStats(player);
    auto playerStats = std::make_shared<PlayerStats>(player);
    playerStatsMap.emplace(playerStats->getUuid(), playerStats);
}

void onLeft(ServerPlayer& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::play_time, player.mTickCount);
    playerStats->addCustomStats(CustomType::leave_game);
    playerStats->saveData();
    // delete playerStats;
    playerStatsMap.erase(uuid);
}

void onSneaking(Player& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->startSneaking();
}

void onSneaked(Player& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addSneakTick();
}

void onTakeItem(Player& player, ItemStack& item) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addStats(StatsDataType::picked_up, item.getTypeName(), item.mCount);
}

void onDropItem(Player* player, ItemStack const& item) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::drop);
    playerStats->addStats(StatsDataType::dropped, item.getTypeName(), item.mCount);
}

void onDied(Player& player, ActorDamageSource const& source) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::deaths);
    // resetCustomSinceTime TODO
    if (!source.isEntitySource()) return;
    Actor* actor = nullptr;
    actor        = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
    if (!actor) return;
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    playerStats->addStats(StatsDataType::killed_by, actor->getTypeName());
}

void onKillMob(Player& player, Mob& mob) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    if (mob.hasType(::ActorType::Player)) {
        playerStats->addCustomStats(CustomType::player_kills);
    } else {
        playerStats->addCustomStats(CustomType::mob_kills);
    }
    playerStats->addStats(StatsDataType::killed, mob.getTypeName());
}

void onTakenDamage(Player* player, float damage, float afterDamage) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;

    auto  heath          = player->getMutableAttribute(SharedAttributes::HEALTH()).mInstance->mCurrentValue;
    auto  absorption     = player->getMutableAttribute(SharedAttributes::ABSORPTION()).mInstance->mCurrentValue;
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
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;

    // auto  heath          = mob->getMutableAttribute(SharedAttributes::HEALTH())->getCurrentValue();
    // auto  absorption     = mob->getMutableAttribute(SharedAttributes::ABSORPTION())->getCurrentValue();
    auto  heath          = mob->getMutableAttribute(SharedAttributes::HEALTH()).mInstance->mCurrentValue;
    auto  absorption     = mob->getMutableAttribute(SharedAttributes::ABSORPTION()).mInstance->mCurrentValue;
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
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
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
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    auto id = instance.getId();
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
        //if (instance.getComponentItem()->isMusicDisk()) playerStats->addCustomStats(CustomType::play_record);
        if((id>=567&&id<=578)||id==657||id==663||id==673||id==736||(id>=782&&id<=784)){
            playerStats->addCustomStats(CustomType::play_record);
        }
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
    auto effectId      = effect.mId;
    auto durationValue = effect.mDuration->mValue;
    auto uuid          = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (effectId == 29 && durationValue == 40 * 60 * 20) {
        playerStats->addCustomStats(CustomType::raid_win);
    } else if (effectId == 36 && durationValue == 30 * 20) {
        playerStats->addCustomStats(CustomType::raid_trigger);
    }
}

void onStartSleep(Player* player) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::sleep_in_bed);
}

void onBlockUsingShield(Player* player, float damage) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::damage_blocked_by_shield, static_cast<int>(damage * 10));
    return;
}

void onOpenTrading(Player* player) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::talked_to_villager);
}

void onJump(Player& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
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
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
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

void onBreedAnimal(mce::UUID uuid) {
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::animals_bred);
}
} // namespace player
} // namespace event
} // namespace stats