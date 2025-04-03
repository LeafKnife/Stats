#include "lk/stats/event/Events.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
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
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/level/Level.h>

#include "lk/stats/event/BlockEventHandle.h"
#include "lk/stats/event/PlayerEventHandle.h"


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
ll::event::ListenerPtr playerPlacedBlockListener;
ll::event::ListenerPtr mobDieListener;
} // namespace

void listenEvents() {
    auto& eventBus = ll::event::EventBus::getInstance();

    // PlayerJoin
    playerJoinListener =
        eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
            player::onJoin(event.self());
        });

    // PlayerDisconnect
    playerDisconnectListener =
        eventBus.emplaceListener<ll::event::player::PlayerDisconnectEvent>([](ll::event::PlayerDisconnectEvent& event) {
            player::onLeft(event.self());
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
            player::onTakeItem(event.self(), event.itemActor().item());
        }
    );

    // PlayerDie
    playerDieListener =
        eventBus.emplaceListener<ll::event::player::PlayerDieEvent>([](ll::event::player::PlayerDieEvent& event) {
            player::onDied(event.self(), event.source());
        });

    // PlayerJump
    playerJumpListener =
        eventBus.emplaceListener<ll::event::player::PlayerJumpEvent>([](ll::event::player::PlayerJumpEvent& event) {
            player::onJump(event.self());
        });

    playerSneakingListener =
        eventBus.emplaceListener<ll::event::player::PlayerSneakingEvent>([](ll::event::PlayerSneakingEvent& event) {
            player::onSneaking(event.self());
        });

    playerSneakedListener =
        eventBus.emplaceListener<ll::event::PlayerSneakedEvent>([](ll::event::PlayerSneakedEvent& event) {
            player::onSneaked(event.self());
        });

    playerSprintingListener =
        eventBus.emplaceListener<ll::event::PlayerSprintingEvent>([](ll::event::PlayerSprintingEvent& event) {
            player::onSprinting(event.self());
        });

    playerSprintedListener =
        eventBus.emplaceListener<ll::event::PlayerSprintedEvent>([](ll::event::PlayerSprintedEvent& event) {
            player::onSprinted(event.self());
        });

    mobDieListener =
        eventBus.emplaceListener<ll::event::entity::MobDieEvent>([](ll::event::entity::MobDieEvent& event) {
            auto& mob    = event.self();
            auto& source = event.source();
            if (!source.isEntitySource()) return;
            Actor* actor = nullptr;
            actor        = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
            if (!actor) return;
            if (source.isChildEntitySource()) {
                actor = actor->getOwner();
            }
            if (actor->getTypeName() != "minecraft:player") return;
            auto& player = *static_cast<::Player*>(actor);
            player::onKillMob(player, mob);
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
    eventBus.removeListener(playerDieListener);
    eventBus.removeListener(playerJumpListener);
    eventBus.removeListener(playerPlacedBlockListener);
    eventBus.removeListener(mobDieListener);
}

} // namespace event
}; // namespace stats