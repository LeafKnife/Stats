//TODO

// LL_TYPE_INSTANCE_HOOK(
//     ItemStackBaseHurtAndBreak,
//     HookPriority::Normal,
//     ItemStackBase,
//     &ItemStackBase::hurtAndBreak,
//     bool,
//     int      deltaDamage,
//     ::Actor* owner
// ) {
//     if (!owner->hasType(::ActorType::Player)) return origin(deltaDamage, owner);
//     ItemStackBase* item = this;
//     auto           res  = origin(deltaDamage, owner);
//     // logger.info(
//     //     "ItemStackBaseHurtAndBreak {} {} {} {}",
//     //     this->getTypeName(),
//     //     deltaDamage,
//     //     owner->getTypeName(),
//     //     res
//     // );
//     if (!res) return res;
//     Player* player = owner->getWeakEntity().tryUnwrap<Player>();
//     if (!player) return res;
//     auto& playerStatsMap = Stats::event::getPlayerStatsMap();
//     auto  uuid           = player->getUuid();
//     auto  playerStats    = playerStatsMap.find(uuid)->second;
//     if (!playerStats) return res;
//     if (!item->isDamageableItem()) return res;
//     auto maxDamage = item->getMaxDamage();
//     auto damage    = item->getDamageValue();
//     if (damage + deltaDamage > maxDamage) {
//         playerStats->addStats(StatsDataType::broken, item->getTypeName());
//     }
//     return res;
// }