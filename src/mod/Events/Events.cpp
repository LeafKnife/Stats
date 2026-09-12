#include "mod/Events/Events.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/entity/ActorHurtEvent.h>
#include <ll/api/event/entity/MobDieEvent.h>
#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerDieEvent.h>
#include <ll/api/event/player/PlayerDisconnectEvent.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerJumpEvent.h>
#include <ll/api/event/player/PlayerPickUpItemEvent.h>
#include <ll/api/event/player/PlayerSneakEvent.h>
#include <ll/api/event/player/PlayerSprintEvent.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/AttributeInstanceConstRef.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/effect/MobEffect.h>
#include <mc/world/effect/MobEffectInstance.h>
#include <mc/world/level/Level.h>

#include "mod/Events/BlockEventHandle.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/Handlers/PlayerStatsHandlers.h"


namespace stats {
namespace event {
namespace {
ll::event::ListenerPtr playerJoinListener;
ll::event::ListenerPtr playerDisconnectListener;
ll::event::ListenerPtr playerDestroyBlockListener;
ll::event::ListenerPtr playerPickUpItemListener;
ll::event::ListenerPtr playerDieListener;
ll::event::ListenerPtr playerJumpListener;
ll::event::ListenerPtr playerSneakingListener;
ll::event::ListenerPtr playerSneakedListener;
ll::event::ListenerPtr playerSprintingListener;
ll::event::ListenerPtr playerSprintedListener;
ll::event::ListenerPtr actorHurtListener;
ll::event::ListenerPtr mobDieListener;
} // namespace

void listenEvents() {
    auto& eventBus = ll::event::EventBus::getInstance();

    // PlayerJoin
    playerJoinListener =
        eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
            handler::onPlayerJoin(event.self());
        });

    // PlayerDisconnect
    playerDisconnectListener =
        eventBus.emplaceListener<ll::event::player::PlayerDisconnectEvent>([](ll::event::PlayerDisconnectEvent& event) {
            handler::onPlayerDisconnect(event.self());
        });

    // PlayerDestroyBlock
    playerDestroyBlockListener = eventBus.emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
        [](ll::event::player::PlayerDestroyBlockEvent& event) {
            block::onBlockDestroyedByPlayer(event.pos(), event.self());
        }
    );

    // PlayerPickUpItem
    playerPickUpItemListener = eventBus.emplaceListener<ll::event::player::PlayerPickUpItemEvent>(
        [](ll::event::player::PlayerPickUpItemEvent& event) {
            handler::onPlayerPickUpItem(event.self(), event.itemActor().item());
        }
    );

    // PlayerDie
    playerDieListener =
        eventBus.emplaceListener<ll::event::player::PlayerDieEvent>([](ll::event::player::PlayerDieEvent& event) {
            handler::onPlayerDied(event.self(), event.source());
        });

    // PlayerJump
    playerJumpListener =
        eventBus.emplaceListener<ll::event::player::PlayerJumpEvent>([](ll::event::player::PlayerJumpEvent& event) {
            handler::onPlayerJump(event.self());
        });

    playerSneakingListener =
        eventBus.emplaceListener<ll::event::player::PlayerSneakingEvent>([](ll::event::PlayerSneakingEvent& event) {
            handler::onPlayerStartSneaking(event.self());
        });

    playerSneakedListener =
        eventBus.emplaceListener<ll::event::PlayerSneakedEvent>([](ll::event::PlayerSneakedEvent& event) {
            handler::onPlayerStopSneaking(event.self());
        });

    playerSprintingListener =
        eventBus.emplaceListener<ll::event::PlayerSprintingEvent>([](ll::event::PlayerSprintingEvent& event) {
            handler::onPlayerStartSprinting(event.self());
        });

    playerSprintedListener =
        eventBus.emplaceListener<ll::event::PlayerSprintedEvent>([](ll::event::PlayerSprintedEvent& event) {
            handler::onPlayerStopSprinting(event.self());
        });

    actorHurtListener = eventBus.emplaceListener<ll::event::entity::ActorHurtEvent>(
        [](ll::event::entity::ActorHurtEvent& event) {
            if (event.isCancelled()) return;

            auto&       victim = event.self();
            auto const& source = event.source();
            auto const  damage = event.damage();
            auto const  absorptionAttribute = victim.getAttribute(SharedAttributes::ABSORPTION());
            auto const  absorption = absorptionAttribute.mPtr ? absorptionAttribute.mPtr->mCurrentValue : 0.0f;
            auto const* resistance = victim.getEffect(*MobEffect::DAMAGE_RESISTANCE());

            getLogger().info(
                "[ActorHurt] victim={} tag='{}' final_damage={:.3f} health_before={} absorption_before={:.3f} "
                "resistance_amplifier={} resistance_applies={} entity_source={} child_source={}",
                victim.getTypeName(),
                victim.getNameTag(),
                damage,
                victim.getHealth(),
                absorption,
                resistance ? resistance->mAmplifier : -1,
                source.isReducedByResistanceEffect(),
                source.isEntitySource(),
                source.isChildEntitySource()
            );

            if (victim.isType(ActorType::Player)) {
                if (auto* player = victim.getEntityContext().getWeakRef().tryUnwrap<Player>().as_ptr()) {
                    handler::onPlayerTakenDamage(player, damage);
                }
            }

            // Preserve the previous Hook's scope: direct player damage only.
            if (!source.isEntitySource() || source.isChildEntitySource()) return;

            auto level = ll::service::getLevel();
            if (!level) return;

            auto* attacker = level->fetchEntity(source.getDamagingEntityUniqueID(), false);
            if (!attacker || !attacker->isType(ActorType::Player)) return;

            getLogger().info(
                "[ActorHurt] attacker={} victim={} damage={:.3f}",
                attacker->getTypeName(),
                victim.getTypeName(),
                damage
            );

            auto* player = attacker->getEntityContext().getWeakRef().tryUnwrap<Player>().as_ptr();
            auto* mob    = victim.getEntityContext().getWeakRef().tryUnwrap<Mob>().as_ptr();
            if (!player || !mob) return;

            handler::onPlayerDealtDamage(mob, player, damage);
        },
        ll::event::EventPriority::Lowest
    );

    mobDieListener =
        eventBus.emplaceListener<ll::event::entity::MobDieEvent>([](ll::event::entity::MobDieEvent& event) {
            auto& mob    = event.self();
            auto& source = event.source();
            if (!source.isEntitySource()) return;
            Actor* actor = nullptr;
            actor        = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
            if (actor) {
                if (source.isChildEntitySource()) actor = actor->getOwner();
            }
            if (!actor) return;
            if (!actor->isType(ActorType::Player)) return;
            auto& player = *static_cast<::Player*>(actor);
            handler::onPlayerKillMob(player, mob);
        });
}

void removeEvents() {
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.removeListener(playerJoinListener);
    eventBus.removeListener(playerDisconnectListener);
    eventBus.removeListener(playerDestroyBlockListener);
    eventBus.removeListener(playerPickUpItemListener);
    eventBus.removeListener(playerSneakingListener);
    eventBus.removeListener(playerSneakedListener);
    eventBus.removeListener(playerSprintingListener);
    eventBus.removeListener(playerSprintedListener);
    eventBus.removeListener(actorHurtListener);
    eventBus.removeListener(playerDieListener);
    eventBus.removeListener(playerJumpListener);
    eventBus.removeListener(mobDieListener);
}

} // namespace event
}; // namespace stats
