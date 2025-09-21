#include "mod/Events/PlayerEventHandle.h"

#include <cmath>
#include <cstddef>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/packet/PlayerAuthInputPacket.h>
#include <mc/world/actor/ActorType.h>
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
    playerStats->stopSneaking();
}

void onSprinting(Player& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->mDistanceCache.isSprinting = true;
}
void onSprinted(Player& player) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->mDistanceCache.isSprinting = false;
}

void onStartRiding(mce::UUID uuid, Actor& vehicle, bool forceRiding) {
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->mDistanceCache.ride = 0;
}
void onStopRiding(mce::UUID uuid, Actor* vehicle) {
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    auto value = playerStats->mDistanceCache.ride;
    if (!vehicle) {
        playerStats->mDistanceCache.ride = 0;
        return;
    }
    if (vehicle->hasCategory(::ActorCategory::BoatRideable)) {
        playerStats->addCustomStats(CustomType::boat_one_cm, value);
    } else if (vehicle->hasCategory(::ActorCategory::MinecartRidable)) {
        playerStats->addCustomStats(CustomType::minecart_one_cm, value);
    } else if (vehicle->hasType(::ActorType::Horse)) {
        playerStats->addCustomStats(CustomType::horse_one_cm, value);
    } else if (vehicle->hasType(::ActorType::Pig)) {
        playerStats->addCustomStats(CustomType::pig_one_cm, value);
    } else if (vehicle->hasType(::ActorType::Strider)) {
        playerStats->addCustomStats(CustomType::strider_one_cm, value);
    } else if (vehicle->hasType(ActorType::HappyGhast)) {
        playerStats->addCustomStats(CustomType::happy_ghast_one_cm, value);
    }
}

void onAuthInput(ServerPlayer& player, PlayerAuthInputPacket const& packet) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    auto pos   = const_cast<Vec3&>(player.getPosition());
    auto dimId = player.getDimensionId().id;
    if (playerStats->mLastDimensionId != dimId) {
        playerStats->mLastDimensionId = dimId;
        playerStats->mLastPos         = pos;
        return;
    }
    // packet.mPosDelta

    if (player.isRiding()) {
        auto value = static_cast<uint64_t>(std::round(player.getPosition().distanceTo(playerStats->mLastPos) * 100));
        playerStats->mDistanceCache.ride += value;
        playerStats->mLastPos             = pos;
    } else {
        // 部分情况下可能检测不到玩家停止骑行
        playerStats->mDistanceCache.ride = 0;

        if (player.isInWater()) {
            auto value = static_cast<uint64_t>(std::floor(packet.mPosDelta->length() * 100));
            if (player.isSwimming()) {
                playerStats->addCustomStats(CustomType::swim_one_cm, value);
            } else if (player._isHeadInWater()) {
                playerStats->addCustomStats(CustomType::walk_on_water_one_cm, value);
            } else {
                playerStats->addCustomStats(CustomType::walk_under_water_one_cm, value);
            }
        } else if (player.isFlying()) {
            auto value = static_cast<uint64_t>(std::floor(packet.mPosDelta->length() * 100));
            playerStats->addCustomStats(CustomType::fly_one_cm, value);
        } else if (player.isOnGround()) {
            auto posOffset = Vec3{0, -0.0784, 0};
            auto value     = static_cast<uint64_t>(std::floor(packet.mPosDelta->distanceTo(posOffset) * 100));
            if (playerStats->mDistanceCache.isSneaking) {
                playerStats->mDistanceCache.sneak += value;
            } else if (playerStats->mDistanceCache.isSprinting) {
                playerStats->addCustomStats(CustomType::sprint_one_cm, value);
            } else {
                playerStats->addCustomStats(CustomType::walk_one_cm, value);
            }
        } else if (player.onClimbableBlock()) {
            auto valueY = packet.mPosDelta->y + 0.0784;
            auto value2 = static_cast<uint64_t>(std::floor(valueY > 0 ? valueY * 100 : 0));
            playerStats->addCustomStats(CustomType::climb_one_cm, value2);
        } else if (playerStats->mDistanceCache.isGliding) {
            auto value = static_cast<uint64_t>(std::floor(packet.mPosDelta->length() * 100));
            playerStats->addCustomStats(CustomType::aviate_one_cm, value);
        }
    }
    if (packet.mInputData->test((size_t)PlayerAuthInputPacket::InputData::StartGliding)) {
        playerStats->mDistanceCache.isGliding = true;
    } else if (packet.mInputData->test((size_t)PlayerAuthInputPacket::InputData::StopGliding)) {
        playerStats->mDistanceCache.isGliding = false;
    }
}

void onTakeItem(Player& player, ItemStack& item) {
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addStats(StatsType::picked_up, item.getTypeName(), item.mCount);
}

void onDropItem(Player* player, ItemStack const& item) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::drop);
    playerStats->addStats(StatsType::dropped, item.getTypeName(), item.mCount);
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
    actor        = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (!actor) return;
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    if (!actor) return;
    playerStats->addStats(StatsType::killed_by, actor->getTypeName());
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
    playerStats->addStats(StatsType::killed, mob.getTypeName());
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

void onCraftedItem(mce::UUID uuid, std::string itemType, int amount, SharedTypes::Legacy::ContainerType type) {
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    switch (type) {
    case SharedTypes::Legacy::ContainerType::Trade:
        playerStats->addStats(StatsType::crafted, itemType, amount);
        playerStats->addCustomStats(CustomType::traded_with_villager);
        break;
    case SharedTypes::Legacy::ContainerType::Enchantment:
        playerStats->addCustomStats(CustomType::enchant_item);
        break;
    case SharedTypes::Legacy::ContainerType::Workbench:
    case SharedTypes::Legacy::ContainerType::Inventory:
        playerStats->addStats(StatsType::crafted, itemType, amount);
        break;
    default:
        break;
    }
}

void onItemHurtAndBroken(Player* player, ItemStackBase* item, int deltaDamage) {
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    if (!item->isDamageableItem()) return;
    if (!item->isArmorItem()) playerStats->addStats(StatsType::used, item->getTypeName());
    auto maxDamage = item->getMaxDamage();
    auto damage    = item->getDamageValue();
    if (damage + deltaDamage > maxDamage) {
        playerStats->addStats(StatsType::broken, item->getTypeName());
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
    switch (itemUseMethod) {
    case ItemUseMethod::Eat:
    case ItemUseMethod::Consume:
    case ItemUseMethod::Throw:
    case ItemUseMethod::FillBottle:
    case ItemUseMethod::PourBucket:
        playerStats->addStats(StatsType::used, instance.getTypeName());
        break;
    case ItemUseMethod::Place:
        playerStats->addStats(StatsType::used, instance.getTypeName());
        // 暂时先用ID, 装addon不知道会不会有问题？
        // if (auto id = instance.getId();
        //     (id >= 567 && id <= 578) || id == 657 || id == 663 || id == 673 || id == 736 || (id >= 782 && id <= 784))
        //     { playerStats->addCustomStats(CustomType::play_record);
        // }
        if (instance.getItem()->isMusicDisk()) {
            playerStats->addCustomStats(CustomType::play_record);
        }
        break;
    case ItemUseMethod::Interact:
        // 交互只记录骨粉，其他与实体交互均不记录
        if (instance.getTypeName() == "minecraft:bone_meal")
            playerStats->addStats(StatsType::used, instance.getTypeName());
        break;
    default:
        break;
    }
};


void onEffectAdded(Player* player, MobEffectInstance const& effect) {
    auto effectId      = effect.mId;
    auto durationValue = effect.mDuration->mValue;
    auto uuid          = player->getUuid();
    auto findPlayer    = playerStatsMap.find(uuid);
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
            playerStats->addStats(StatsType::crafted, oldItem.getTypeName());
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
void onFishCaught(mce::UUID uuid) {
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::fish_caught);
}
} // namespace player
} // namespace event
} // namespace stats