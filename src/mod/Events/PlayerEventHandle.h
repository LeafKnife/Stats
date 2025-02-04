#pragma once

#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffectInstance.h"

namespace stats {
namespace event {
namespace player {
void onJoin(Player& player);
void onLeft(Player& player);
void onTakeItem(Player& player, ItemStack& item);
void onDropItem(Player* player, ItemStack const& item);
void onDied(Player& player, ActorDamageSource const& source);
void onKillMob(Player& player, Mob& mob);
//void onHurt(Mob* mob, float damage, float afterDamage);
//void onMobHurtByPlayer(Mob* mob, Actor* damageSource, float damage, float afterDamage);
void onCraftedItem();
void onBrokenItem();
void onUsedItem();
void onEffectAdded(Player* player, MobEffectInstance const& effect);
void onStartSleep(Player* player);
void onBlockUsingShield(Player* player,float damage);
void onOpenTrading(Player* player);
void onJump(Player& player);
void onMove();
} // namespace player
} // namespace event
} // namespace stats