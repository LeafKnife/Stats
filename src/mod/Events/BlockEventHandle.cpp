#include "mod/Events/BlockEventHandle.h"

#include <cstdint>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>

#include "mod/Stats/Stats.h"

namespace stats {
namespace event {
namespace block {
namespace {
auto& playerStatsMap = getPlayerStatsMap();
}

const Block& getBlockByBlockPos(BlockPos const& pos, DimensionType id) {
    auto  dimension = ll::service::getLevel()->getDimension(id).lock();
    auto& block     = dimension->getBlockSourceFromMainChunkSource().getBlock(pos);
    return block;
}

void onBlockDestroyedByPlayer(BlockPos const& pos, Player& player) {
    if (player.isSimulatedPlayer()) return;
    if (player.isCreative()) return;
    auto  uuid       = player.getUuid();
    auto& bl         = getBlockByBlockPos(pos, player.getDimensionId());
    auto  findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addStats(StatsType::mined, bl.getTypeName());
}

void onBlockPlacedByPlayer(BlockPos const& pos, Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto& bl         = getBlockByBlockPos(pos, player.getDimensionId());
    auto  uuid       = player.getUuid();
    auto  findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addStats(StatsType::used, bl.getTypeName());
}

void onBlockUsed(BlockPos const& pos, Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto& block      = getBlockByBlockPos(pos, player.getDimensionId());
    auto  blockType  = block.getTypeName();
    auto  uuid       = player.getUuid();
    auto  findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    auto it = CustomInteractBlockMap.find(blockType);
    if (it == CustomInteractBlockMap.end()) return;
    playerStats->addCustomStats(it->second);
    return;
}

void onNoteBlockAttacked(Player* player) {
    if (player->isSimulatedPlayer()) return;
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::play_noteblock);
    return;
}

void onCakeBlockRemovedSlice(Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::eat_cake_slice);
    return;
}

void onCauldronBlockUseInventory(Player& player, ItemStack& currentItem, ItemStack& replaceItem, int useCount) {
    if (player.isSimulatedPlayer()) return;
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    // JE原版装"水"(用桶装水 岩浆 细雪) 取"水"(桶、空瓶 水瓶交互)
    // 此处后续待修改
    if (currentItem.getTypeName() == "minecraft:bucket") {
        playerStats->addCustomStats(CustomType::use_cauldron);
    } else if (currentItem.getTypeName() == "minecraft:water_bucket") {
        playerStats->addCustomStats(CustomType::fill_cauldron);
    }
    return;
}

void onCauldronBlockClean(
    Player const&                                player,
    short                                        itemId,
    ::MinecraftEventing::POIBlockInteractionType interactionType
) {
    if (player.isSimulatedPlayer()) return;
    if (interactionType != ::MinecraftEventing::POIBlockInteractionType::ClearItem) return;
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    // 潜影盒
    if ((itemId >= -627 && itemId <= -613) || itemId == 218)
        return playerStats->addCustomStats(CustomType::clean_shulker_box);
    // 皮革盔甲
    if (itemId >= 360 && itemId <= 363) return playerStats->addCustomStats(CustomType::clean_armor);
    // 旗帜
    if (itemId == 600) return playerStats->addCustomStats(CustomType::clean_banner);
}

void onFlowerPotBlockPlaceFlower(Player& player) {
    if (player.isSimulatedPlayer()) return;
    auto uuid       = player.getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::pot_flower);
    return;
}

void onProjectileHitTargetBlock(Actor const& projectile) {
    auto mob = projectile.getOwner();
    if (!mob||!mob->isType(::ActorType::Player)) return;
    Player* player = mob->getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return;
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::target_hit);
    return;
}
void onFallOn(Actor& actor, float fallDistance) {
    if (!actor.isType(::ActorType::Player)) return;
    Player* player = actor.getEntityContext().getWeakRef().tryUnwrap<Player>();
    if (!player) return;
    auto uuid       = player->getUuid();
    auto findPlayer = playerStatsMap.find(uuid);
    if (findPlayer == playerStatsMap.end()) return;
    auto playerStats = findPlayer->second;
    if (!playerStats) return;
    uint64_t value = static_cast<uint64_t>(fallDistance * 100);
    playerStats->addCustomStats(CustomType::fall_one_cm, value);
}
} // namespace block
} // namespace event
} // namespace stats