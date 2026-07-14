#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/item/ItemUseMethod.h>
#include <mc/world/item/components/ComponentItem.h>

#include "mod/Stats/Stats.h"

namespace stats::handler {

void onPlayerPickUpItem(Player& player, ItemStack& item) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addStats(StatsType::picked_up, item.getTypeName(), item.mCount);
}

void onPlayerDropItem(Player* player, ItemStack const& item) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::drop);
    playerStats->addStats(StatsType::dropped, item.getTypeName(), item.mCount);
}

void onPlayerCraftedItem(mce::UUID uuid, std::string itemType, int amount, SharedTypes::Legacy::ContainerType type) {
    auto* playerStats = findPlayerStats(uuid);
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

void onPlayerItemHurtAndBroken(Player* player, ItemStackBase* item, int deltaDamage) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats || !item->isDamageableItem()) return;
    if (!item->isArmorItem()) playerStats->addStats(StatsType::used, item->getTypeName());

    auto const maxDamage = item->getMaxDamage();
    auto const damage    = item->getDamageValue();
    if (damage + deltaDamage > maxDamage) {
        playerStats->addStats(StatsType::broken, item->getTypeName());
    }
}

void onPlayerUsedItem(Player* player, ItemStackBase& instance, ItemUseMethod itemUseMethod, bool consumeItem) {
    // Several entity interactions and special items require dedicated hooks.
    if (!consumeItem) return;
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
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
        if (instance.getItem()->isMusicDisk()) {
            playerStats->addCustomStats(CustomType::play_record);
        }
        break;
    case ItemUseMethod::Interact:
        if (instance.getTypeName() == "minecraft:bone_meal") {
            playerStats->addStats(StatsType::used, instance.getTypeName());
        }
        break;
    default:
        break;
    }
}

void onPlayerChangeContainerWith(
    Player&          player,
    std::string      blockType,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    if (blockType != "minecraft:furnace" && blockType != "minecraft:lit_furnace"
        && blockType != "minecraft:smoker") {
        return;
    }
    if (slot != 2 || oldItem.isNull()) return;
    if (oldItem.mCount > newItem.mCount) {
        playerStats->addStats(StatsType::crafted, oldItem.getTypeName());
    }
}

} // namespace stats::handler
