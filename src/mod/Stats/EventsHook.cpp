#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ThreadName.h"

#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/ContainerID.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageCause.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/ArmorStand.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/Hopper.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/BedSleepingResult.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/events/BlockEventCoordinator.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/events/PlayerEventListener.h"
#include "mc/world/events/PlayerOpenContainerEvent.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/CraftingContainer.h"
#include "mc/world/inventory/network/crafting/CraftHandlerCrafting.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ShieldItem.h"
#include "mc/world/item/TridentItem.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/BasePressurePlateBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/CakeBlock.h"
#include "mc/world/level/block/CampfireBlock.h"
#include "mc/world/level/block/CauldronBlock.h"
#include "mc/world/level/block/ComparatorBlock.h"
#include "mc/world/level/block/CraftingTableBlock.h"
#include "mc/world/level/block/DiodeBlock.h"
#include "mc/world/level/block/FarmBlock.h"
#include "mc/world/level/block/FlowerPotBlock.h"
#include "mc/world/level/block/ItemFrameBlock.h"
#include "mc/world/level/block/LiquidBlockDynamic.h"
#include "mc/world/level/block/NoteBlock.h"
#include "mc/world/level/block/RedStoneWireBlock.h"
#include "mc/world/level/block/RedstoneTorchBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/TargetBlock.h"
#include "mc/world/level/block/actor/BarrelBlockActor.h"
#include "mc/world/level/block/actor/BaseCommandBlock.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/IdentityDefinition.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/ScoreboardId.h"
#include "mc/world/scores/ServerScoreboard.h"

#include "mod/MyMod.h"
#include "mod/Stats/CustomStatsType.h"
#include "mod/Stats/Events.h"
#include "mod/Stats/PlayerStats.h"

namespace Stats::event::hook {
auto& logger = Stats::MyMod().getSelf().getLogger();
LL_TYPE_INSTANCE_HOOK(
    PlayerStartSleepHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& pos
) {
    auto    res            = origin(pos);
    Player* player         = this;
    auto&   playerStatsMap = Stats::event::getPlayerStatsMap();
    auto    uuid           = player->getUuid();
    auto    playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    if (res == BedSleepingResult::Ok) {
        playerStats->addCustomStats(Stats::CustomType::sleep_in_bed);
    }
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    CraftingTableUseHook,
    HookPriority::Normal,
    CraftingTableBlock,
    &CraftingTableBlock::$use,
    bool,
    ::Player&         player,
    ::BlockPos const& pos,
    uchar             face
) {
    auto res = origin(player, pos, face);
    // logger.info("CraftingTableBlockUseHook {} {} {}", player.getRealName(), pos, res);
    if (!res) return res;
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    playerStats->addCustomStats(CustomType::interact_with_crafting_table);
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    NoteBlockAttackHook,
    HookPriority::Normal,
    NoteBlock,
    &NoteBlock::$attack,
    bool,
    ::Player*         player,
    ::BlockPos const& pos
) {
    auto res = origin(player, pos);
    // logger.info("NoteBlockAttackHook {} {} {}", player->getRealName(), pos, res);
    if (!res) return res;
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player->getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    playerStats->addCustomStats(CustomType::play_noteblock);
    return res;
}

LL_TYPE_STATIC_HOOK(
    CakeRemoveSliceHook,
    HookPriority::Normal,
    CakeBlock,
    &CakeBlock::removeCakeSlice,
    void,
    ::Player&         player,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Block const*    cakeBlock
) {
    origin(player, region, pos, cakeBlock);
    // logger.info("CakeRemoveSliceHook {} {}", player.getRealName(), pos);
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::eat_cake_slice);
    return;
}

LL_TYPE_INSTANCE_HOOK(
    CauldronBlockUseInventoryHook,
    HookPriority::Normal,
    CauldronBlock,
    &CauldronBlock::_useInventory,
    void,
    class Player&    player,
    class ItemStack& current,
    class ItemStack& replaceWith,
    int              useCount
) {
    // logger.info(
    //     "CauldronBlockUseInventoryHook {} {} {} {}",
    //     player.getRealName(),
    //     current.getTypeName(),
    //     replaceWith.getTypeName(),
    //     useCount
    // );
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return origin(player, current, replaceWith, useCount);
    if (current.getTypeName() == "minecraft:bucket") playerStats->addCustomStats(CustomType::use_cauldron);
    if (current.getTypeName() == "minecraft:water_bucket") playerStats->addCustomStats(CustomType::fill_cauldron);
    return origin(player, current, replaceWith, useCount);
}

LL_TYPE_INSTANCE_HOOK(
    CauldronBlockCleanHook,
    HookPriority::Normal,
    CauldronBlock,
    &CauldronBlock::_sendCauldronUsedEventToClient,
    void,
    class Player const&                          player,
    short                                        itemId,
    ::MinecraftEventing::POIBlockInteractionType interactionType
) {
    origin(player, itemId, interactionType);
    // logger.info("CauldronBlockCleanHook {} {} {}", player.getRealName(), itemId, static_cast<int>(interactionType));
    if (interactionType != ::MinecraftEventing::POIBlockInteractionType::ClearItem) return;
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    if ((itemId >= -627 && itemId <= -613) || itemId == 218)
        return playerStats->addCustomStats(CustomType::clean_shulker_box);
    if (itemId >= 360 && itemId <= 363) return playerStats->addCustomStats(CustomType::clean_armor);
    if (itemId == 600) return playerStats->addCustomStats(CustomType::clean_banner);
}

LL_TYPE_INSTANCE_HOOK(
    FlowerPotBlockTryPlaceFlowerHook,
    HookPriority::Normal,
    FlowerPotBlock,
    &FlowerPotBlock::_tryPlaceFlower,
    bool,
    ::Player&         player,
    ::BlockPos const& blockPos
) {
    auto res = origin(player, blockPos);
    // logger.info("FlowerPotBlockTryPlaceFlowerHook {} {} {}", player.getRealName(), blockPos, res);
    if (!res) return res;
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    playerStats->addCustomStats(CustomType::pot_flower);
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    CampfireBlockUseHook,
    HookPriority::Normal,
    CampfireBlock,
    &CampfireBlock::$use,
    bool,
    ::Player&         player,
    ::BlockPos const& pos,
    uchar             face
) {
    auto res = origin(player, pos, face);
    // logger.info("CampfireBlockUseHook {} {} {}", player.getRealName(), pos, res);
    if (!res) return res;
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    playerStats->addCustomStats(CustomType::interact_with_campfire);
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    TargetBlockOnProjectileHitHook,
    HookPriority::Normal,
    TargetBlock,
    &TargetBlock::$onProjectileHit,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor const&    projectile
) {
    auto mob = projectile.getOwner();
    logger.info(
        "TargetBlockOnProjectileHitHook {} {} {} {}",
        region.getBlock(pos).getTypeName(),
        pos,
        projectile.getTypeName(),
        mob->getTypeName()
    );
    if (!mob->isType(::ActorType::Player)) return origin(region, pos, projectile);
    Player* player = mob->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return origin(region, pos, projectile);
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player->getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return origin(region, pos, projectile);
    playerStats->addCustomStats(CustomType::target_hit);
    return origin(region, pos, projectile);
}

LL_TYPE_INSTANCE_HOOK(
    BlockInteractedWithHook,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockInteractedWith,
    void,
    ::Player&         player,
    ::BlockPos const& blockPos
) {
    origin(player, blockPos);
    // auto& logger         = Stats::MyMod().getSelf().getLogger();
    auto& block          = Stats::event::getBlockByBlockPos(blockPos, player.getDimensionId());
    auto  blockType      = block.getTypeName();
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    // logger.debug("BlockInteractedWith {} {} {}", player.getRealName(), blockType, blockPos);
    auto it = CustomInteractBlockMap.find(blockType);
    if (it != CustomInteractBlockMap.end()) {
        return playerStats->addCustomStats(it->second);
    }
    if (blockType == "minecraft:flower_pot") {
        // TODO
    }
    // 营火 唱片机 工作台~ 监听不到
}

LL_TYPE_INSTANCE_HOOK(PlayerEatHook, HookPriority::Normal, Player, &Player::eat, void, ItemStack const& instance) {
    Player* player         = this;
    auto    item           = &const_cast<ItemStack&>(instance);
    auto    playerStatsMap = Stats::event::getPlayerStatsMap();
    auto    uuid           = player->getUuid();
    auto    playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    playerStats->addStats(StatsDataType::used, item->getTypeName());
    origin(instance);
    // logger.info("PlayerEatHook {} {}", player->getRealName(), item->getTypeName());
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook1,
    HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& item,
    bool             randomly
) {
    auto res            = origin(item, randomly);
    auto player         = this;
    auto playerStatsMap = Stats::event::getPlayerStatsMap();
    auto uuid           = player->getUuid();
    auto playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return res;
    if (res && !randomly) {
        playerStats->addCustomStats(CustomType::drop);
        playerStats->addStats(StatsDataType::dropped, item.getTypeName(), item.mCount);
    }
    // logger.info("PlayerDropItemHook1{} {} {} {}", player->getRealName(), item.getTypeName(), randomly, res);
    return res;
}

void hook() {
    PlayerStartSleepHook::hook();
    PlayerDropItemHook1::hook();
    PlayerEatHook::hook();
    CraftingTableUseHook::hook();
    NoteBlockAttackHook::hook();
    CakeRemoveSliceHook::hook();
    CauldronBlockUseInventoryHook::hook();
    CauldronBlockCleanHook::hook();
    FlowerPotBlockTryPlaceFlowerHook::hook();
    CampfireBlockUseHook::hook();
    TargetBlockOnProjectileHitHook::hook();
    BlockInteractedWithHook::hook();
}
void unhook() {
    PlayerStartSleepHook::unhook();
    PlayerDropItemHook1::unhook();
    PlayerEatHook::unhook();
    CraftingTableUseHook::unhook();
    NoteBlockAttackHook::unhook();
    CraftingTableUseHook::unhook();
    CauldronBlockUseInventoryHook::unhook();
    CauldronBlockCleanHook::unhook();
    FlowerPotBlockTryPlaceFlowerHook::unhook();
    CampfireBlockUseHook::unhook();
    TargetBlockOnProjectileHitHook::unhook();
    BlockInteractedWithHook::unhook();
}
} // namespace Stats::event::hook