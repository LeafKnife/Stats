#pragma once

#include <string>

#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/world/item/ItemUseMethod.h>

class Actor;
class ActorDamageSource;
class ItemStack;
class ItemStackBase;
class Mob;
class MobEffectInstance;
class Player;
class PlayerAuthInputPacket;
class ServerPlayer;

namespace mce {
class UUID;
}

namespace stats::handler {
void onPlayerJoin(Player& player);
void onPlayerDisconnect(ServerPlayer& player);
void onPlayerStartSneaking(Player& player);
void onPlayerStopSneaking(Player& player);
void onPlayerStartSprinting(Player& player);
void onPlayerStopSprinting(Player& player);
void onPlayerStartRiding(Player& player);
void onPlayerStopRiding(mce::UUID uuid, Actor* vehicle);
void onPlayerAuthInput(ServerPlayer& player, PlayerAuthInputPacket const& packet);
void onPlayerPickUpItem(Player& player, ItemStack& item);
void onPlayerDropItem(Player* player, ItemStack const& item);
void onPlayerDied(Player& player, ActorDamageSource const& source);
void onPlayerKillMob(Player& player, Mob& mob);
void onPlayerTakenDamage(Player* player, float finalDamage);
void onPlayerDealtDamage(Mob* mob, Player* player, float finalDamage);
void onPlayerResistedDamage(Player* player, float resistanceDamage);
void onPlayerDealtResistedDamage(Player* player, float resistanceDamage);
void onPlayerCraftedItem(mce::UUID uuid, std::string itemType, int amount, SharedTypes::Legacy::ContainerType type);
void onPlayerItemHurtAndBroken(Player* player, ItemStackBase* item, int deltaDamage);
void onPlayerUsedItem(Player* player, ItemStackBase& instance, ItemUseMethod itemUseMethod, bool consumeItem);
void onPlayerEffectAdded(Player* player, MobEffectInstance const& effect);
void onPlayerStartSleep(Player* player);
void onPlayerBlockUsingShield(Player* player, float damage);
void onPlayerOpenTrading(Player* player);
void onPlayerJump(Player& player);
void onPlayerChangeContainerWith(
    Player&          player,
    std::string      blockType,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem
);
void onPlayerBreedAnimal(mce::UUID uuid);
void onPlayerFishCaught(mce::UUID uuid);
} // namespace stats::handler
