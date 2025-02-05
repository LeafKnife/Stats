#pragma once

#include "mc/events/MinecraftEventing.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/Block/Block.h"

namespace stats {
namespace event {
namespace block {
const Block& getBlockByBlockPos(BlockPos const& pos, DimensionType id);
void onBlockDestroyedByPlayer(BlockPos const& pos, Player& player);
void onBlockPlacedByPlayer(BlockPos const& pos,Player& player);
void onBlockInteracted(BlockPos const& pos, Player& player);
void onCraftingTableBlockUsed(Player& player);
void onNoteBlockAttacked(Player* player);
void onCakeBlockRemovedSlice(Player& player);
void onCauldronBlockUseInventory(Player& player, ItemStack& currentItem, ItemStack& replaceItem, int useCount);
void onCauldronBlockClean(
    Player const&                                player,
    short                                        itemId,
    ::MinecraftEventing::POIBlockInteractionType interactionType
);
void onFlowerPotBlockPlaceFlower(Player& player);
void onCampfireBlockUsed(Player& player);
void onProjectileHitTargetBlock(Actor const& projectile);
} // namespace block
} // namespace event
} // namespace Stats