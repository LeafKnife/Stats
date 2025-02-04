//TODO

// LL_TYPE_INSTANCE_HOOK(
//     MobGetDamageAfterResistanceEffectHook,
//     HookPriority::Normal,
//     Mob,
//     &Mob::getDamageAfterResistanceEffect,
//     float,
//     ::ActorDamageSource const& source,
//     float                      damage
// ) {
//     auto afterDamage = origin(source, damage);
//     Mob* mob         = this;

//     if (mob->hasType(::ActorType::Player)) {
//         playerHurt(mob, damage, afterDamage);
//     }
//     if (source.isEntitySource()) {
//         Actor* damageSource = nullptr;
//         if (!source.isEntitySource()) {
//             if (source.isChildEntitySource()) {
//                 damageSource = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
//             } else {
//                 damageSource = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
//             }
//             if (damageSource->hasType(::ActorType::Player)) {
//                 mobHurtByPlayer(mob, damageSource, damage, afterDamage);
//             }
//         }
//     }
//     // logger.info("MobGetDamageAfterResistanceEffectHook {} {} {}", this->getTypeName(), damage * 100, afterDamage *
//     // 100);
//     return afterDamage;
// }