#include "mod/Stats/Events.h"
#include "mc/world/level/BlockPos.h"
#include "mod/Stats/PlayerStats.h"

#include "ll/api/event/ListenerBase.h"

#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/utility/optional_ref.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/block_serialization_utils/BlockSerializationUtils.h"
#include "mc/world/level/block/components/BlockLiquidDetectionComponent.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"

#include "ll/api/event/EventBus.h"
// #include "ll/api/event/command/ExecuteCommandEvent.h"
// #include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/entity/MobDieEvent.h"
// #include "ll/api/event/player/PlayerAddExperienceEvent.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
// #include "ll/api/event/player/PlayerChatEvent.h"
// #include "ll/api/event/player/PlayerClickEvent.h"
// #include "ll/api/event/player/PlayerConnectEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
// #include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerJumpEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
// #include "ll/api/event/player/PlayerRespawnEvent.h"
// #include "ll/api/event/player/PlayerSneakEvent.h"
// #include "ll/api/event/player/PlayerSprintEvent.h"
// #include "ll/api/event/player/PlayerSwingEvent.h"
// #include "ll/api/event/player/PlayerUseItemEvent.h"
// #include "ll/api/event/server/ServerStartedEvent.h"
// #include "ll/api/event/world/BlockChangedEvent.h"
// #include "ll/api/event/world/FireSpreadEvent.h"
// #include "ll/api/event/world/SpawnMobEvent.h"

namespace Stats {
namespace event {
namespace {
PlayerStatsMap         playerStatsMaps;
ll::event::ListenerPtr playerJoinListener;
ll::event::ListenerPtr playerDisconnectListener;
ll::event::ListenerPtr playerDestroyBlockListener;
ll::event::ListenerPtr playerPickUpItemListener;
ll::event::ListenerPtr playerDieListener;
ll::event::ListenerPtr playerJumpListener;
ll::event::ListenerPtr playerPlacedBlockListener;
ll::event::ListenerPtr mobDieListener;
} // namespace
const ::Block& getBlockByBlockPos(const BlockPos& pos, int dimensionId) {
    auto           dimension = ll::service::getLevel()->getDimension(dimensionId);
    const ::Block& block     = dimension->getBlockSourceFromMainChunkSource().getBlock(pos);
    return block;
}
void listenEvents() {
    auto& eventBus = ll::event::EventBus::getInstance();
    // auto& logger   = Stats::MyMod().getSelf().getLogger();
    playerStatsMaps.emplace();

    // PlayerJoin
    playerJoinListener =
        eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
            auto& player = event.self();
            if (player.isSimulatedPlayer()) return;
            PlayerStats* playerStats = new PlayerStats(player);
            playerStatsMaps.emplace(playerStats->getUuid(), playerStats);
        });

    // PlayerDisconnect
    playerDisconnectListener =
        eventBus.emplaceListener<ll::event::player::PlayerDisconnectEvent>([](ll::event::PlayerDisconnectEvent& event) {
            auto& player = event.self();
            if (player.isSimulatedPlayer()) return;
            auto uuid        = player.getUuid();
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addCustomStats(CustomType::leave_game);
            playerStats->saveData();
            delete playerStats;
            playerStatsMaps.erase(uuid);
        });

    // PlayerDestroyBlock
    playerDestroyBlockListener = eventBus.emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
        [](ll::event::player::PlayerDestroyBlockEvent& event) {
            auto& player   = event.self();
            if (player.isSimulatedPlayer()) return;
            if (player.isCreative()) return;
            auto uuid        = player.getUuid();
            auto& blockPos = event.pos();
            auto& bl       = Stats::event::getBlockByBlockPos(blockPos, player.getDimensionId());   
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addStats(StatsDataType::mined, bl.getTypeName());
        }
    );

    // PlayerPickUpItem
    playerPickUpItemListener = eventBus.emplaceListener<ll::event::player::PlayerPickUpItemEvent>(
        [](ll::event::player::PlayerPickUpItemEvent& event) {
            auto& player = event.self();
            if (player.isSimulatedPlayer()) return;
            auto& item   = event.itemActor().item();
            auto uuid        = player.getUuid();
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addStats(StatsDataType::picked_up, item.getTypeName(), item.mCount);
        }
    );

    // PlayerAttack
    // eventBus.emplaceListener<ll::event::player::PlayerAttackEvent>([](ll::event::player::PlayerAttackEvent& event) {
    //     auto& player = event.self();
    //     if (player.isSimulatedPlayer()) return;
    //     auto uuid        = player.getUuid();
    //     auto playerStats = playerStatsMaps.find(uuid)->second;
    //     if (!playerStats) return;
    //     auto& damage = event.cause();
    //     playerStats->addCustomStats(CustomType::damage_dealt, static_cast<int>(damage * 10));
    // });

    // PlayerDie
    playerDieListener =
        eventBus.emplaceListener<ll::event::player::PlayerDieEvent>([](ll::event::player::PlayerDieEvent& event) {
            auto& player = event.self();
            if (player.isSimulatedPlayer()) return;
            auto& source = event.source();
            auto uuid        = player.getUuid();
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addCustomStats(CustomType::deaths);
            // resetCustomSinceTime TODO
            if (!source.isEntitySource()) return;
            Actor* actor = nullptr;
            actor        = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
            if (!actor) return;
            if (source.isChildEntitySource()) {
                actor = actor->getOwner();
            }
            playerStats->addStats(StatsDataType::killed_by, actor->getTypeName());
        });

    // PlayerJump
    playerJumpListener =
        eventBus.emplaceListener<ll::event::player::PlayerJumpEvent>([](ll::event::player::PlayerJumpEvent& event) {
            auto& player = event.self();
            if (player.isSimulatedPlayer()) return;
            auto uuid        = player.getUuid();
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addCustomStats(CustomType::jump);
        });

    // // PlayerSneak
    // eventBus.emplaceListener<ll::event::player::PlayerSneakEvent>(Args && args...);

    // // PlayerSwing
    // eventBus.emplaceListener<ll::event::player::PlayerSwingEvent>(Args && args...);

    // // PlayerUseItem
    // eventBus.emplaceListener<ll::event::player::PlayerUseItemEvent>(Args && args...);

    // PlayerPlaceBlock
    playerPlacedBlockListener = eventBus.emplaceListener<ll::event::player::PlayerPlacedBlockEvent>(
        [](ll::event::player::PlayerPlacedBlockEvent& event) {
            auto& player    = event.self();
            if (player.isSimulatedPlayer()) return;
            auto& BlockPos  = event.pos();
            auto  dimension = ll::service::getLevel()->getDimension(player.getDimensionId());
            auto& bl        = dimension->getBlockSourceFromMainChunkSource().getBlock(BlockPos);
            auto uuid        = player.getUuid();
            auto playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            playerStats->addStats(StatsDataType::used, bl.getTypeName());
        }
    );

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
            auto& player      = *static_cast<::Player*>(actor);
            auto  uuid        = player.getUuid();
            auto  playerStats = playerStatsMaps.find(uuid)->second;
            if (!playerStats) return;
            auto& mobType = mob.getTypeName();
            if (mobType == "minecraft:player") {
                playerStats->addCustomStats(CustomType::player_kills);
            } else {
                playerStats->addCustomStats(CustomType::mob_kills);
                playerStats->addStats(StatsDataType::killed, mobType);
            }
        });
}

void removeEvents() {
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.removeListener(playerJoinListener);
    eventBus.removeListener(playerDisconnectListener);
    eventBus.removeListener(playerDestroyBlockListener);
    eventBus.removeListener(playerPickUpItemListener);
    eventBus.removeListener(playerDieListener);
    eventBus.removeListener(playerJumpListener);
    eventBus.removeListener(playerPlacedBlockListener);
    eventBus.removeListener(mobDieListener);
}

PlayerStatsMap& getPlayerStatsMap() { return playerStatsMaps; }
} // namespace event
}; // namespace Stats