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
#include "mc/world/level/block/ComparatorBlock.h"
#include "mc/world/level/block/DiodeBlock.h"
#include "mc/world/level/block/FarmBlock.h"
#include "mc/world/level/block/ItemFrameBlock.h"
#include "mc/world/level/block/LiquidBlockDynamic.h"
#include "mc/world/level/block/RedStoneWireBlock.h"
#include "mc/world/level/block/RedstoneTorchBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
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
    PlayerUseItemOnHook,
    HookPriority::Normal,
    GameMode,
    &GameMode::$useItemOn,
    InteractionResult,
    ItemStack&      item,
    BlockPos const& blockPos,
    uchar           face,
    Vec3 const&     clickPos,
    Block const*    block,
    bool            isFirstEvent
) {
    auto res = origin(item, blockPos, face, clickPos, block, isFirstEvent);
    if (!isFirstEvent) return res;
    auto& logger = Stats::MyMod().getSelf().getLogger();
    logger
        .info("PlayeruseItemuseItemHook {} {} {}", item.getTypeName(), isFirstEvent,
        static_cast<int>(res.mResult));
    return res;
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
    //auto& logger         = Stats::MyMod().getSelf().getLogger();
    auto& block          = Stats::event::getBlockByBlockPos(blockPos, player.getDimensionId());
    auto  blockType      = block.getTypeName();
    auto& playerStatsMap = Stats::event::getPlayerStatsMap();
    auto  uuid           = player.getUuid();
    auto  playerStats    = playerStatsMap.find(uuid)->second;
    if (!playerStats) return;
    //logger.debug("BlockInteractedWith {} {} {}", player.getRealName(), blockType, blockPos);
    auto it = CustomInteractBlockMap.find(blockType);
    if (it != CustomInteractBlockMap.end()) {
        return playerStats->addCustomStats(it->second);
    }
    if(blockType =="minecraft:flower_pot"){
        //TODO
    }
    if(blockType == "minecraft:cauldron"){
        //TODO
    }
    //营火 唱片机 工作台 监听不到
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
    BlockInteractedWithHook::hook();
}
void unhook() {
    PlayerStartSleepHook::unhook();
    PlayerDropItemHook1::unhook();
    BlockInteractedWithHook::unhook();
}
} // namespace Stats::event::hook