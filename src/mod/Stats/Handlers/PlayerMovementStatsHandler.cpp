#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <cmath>
#include <cstddef>

#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/network/packet/PlayerAuthInputPacket.h>
#include <mc/platform/UUID.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Stats.h"

namespace stats::handler {

void onPlayerStartSneaking(Player& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->startSneaking(ll::service::getLevel()->getCurrentTick().tickID);
}

void onPlayerStopSneaking(Player& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->stopSneaking(ll::service::getLevel()->getCurrentTick().tickID);
}

void onPlayerStartSprinting(Player& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->mDistanceCache.isSprinting = true;
}

void onPlayerStopSprinting(Player& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->mDistanceCache.isSprinting = false;
}

void onPlayerStartRiding(mce::UUID uuid) {
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->mDistanceCache.ride = 0;
}

void onPlayerStopRiding(mce::UUID uuid, Actor* vehicle) {
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto const value = playerStats->mDistanceCache.ride;
    if (!vehicle) {
        playerStats->mDistanceCache.ride = 0;
        return;
    }
    if (vehicle->hasCategory(ActorCategory::BoatRideable)) {
        playerStats->addCustomStats(CustomType::boat_one_cm, value);
    } else if (vehicle->hasCategory(ActorCategory::MinecartRidable)) {
        playerStats->addCustomStats(CustomType::minecart_one_cm, value);
    } else if (vehicle->isType(ActorType::Horse)) {
        playerStats->addCustomStats(CustomType::horse_one_cm, value);
    } else if (vehicle->isType(ActorType::Pig)) {
        playerStats->addCustomStats(CustomType::pig_one_cm, value);
    } else if (vehicle->isType(ActorType::Strider)) {
        playerStats->addCustomStats(CustomType::strider_one_cm, value);
    } else if (vehicle->isType(ActorType::HappyGhast)) {
        playerStats->addCustomStats(CustomType::happy_ghast_one_cm, value);
    } else if (vehicle->isType(ActorType::Nautilus)) {
        playerStats->addCustomStats(CustomType::nautilus_one_cm, value);
    }
    playerStats->mDistanceCache.ride = 0;
}

void onPlayerAuthInput(ServerPlayer& player, PlayerAuthInputPacket const& packet) {
    auto const uuid        = player.getUuid();
    auto*      playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    auto const& position  = player.getPosition();
    auto const& posDelta  = *packet.mPosDelta;
    auto const& inputData = *packet.mInputData;
    auto const  dimId     = player.getDimensionId().mValue;

    auto updateGlidingState = [&] {
        if (inputData.test(static_cast<std::size_t>(PlayerAuthInputPacket::InputData::StartGliding))) {
            playerStats->mDistanceCache.isGliding = true;
        } else if (inputData.test(static_cast<std::size_t>(PlayerAuthInputPacket::InputData::StopGliding))) {
            playerStats->mDistanceCache.isGliding = false;
        }
    };

    if (playerStats->mLastDimensionId != dimId) {
        playerStats->mLastDimensionId = dimId;
        playerStats->mLastPos         = position;
        updateGlidingState();
        return;
    }
    if (player.isRiding(nullptr)) {
        auto const value = static_cast<uint64_t>(std::floor(position.distanceTo(playerStats->mLastPos) * 100));
        playerStats->mDistanceCache.ride += value;
        playerStats->mLastPos             = position;
    } else {
        // Some dismount paths may not emit the stop-riding callback.
        playerStats->mDistanceCache.ride = 0;

        if (player.isInWaterOrRain()) {
            auto const value = static_cast<uint64_t>(std::floor(posDelta.length() * 100));
            if (player.isSwimming()) {
                playerStats->addCustomStats(CustomType::swim_one_cm, value);
            } else if (player.isImmersedInWater()) {
                playerStats->addCustomStats(CustomType::walk_on_water_one_cm, value);
            } else {
                playerStats->addCustomStats(CustomType::walk_under_water_one_cm, value);
            }
        } else if (player.isFlying()) {
            auto const value = static_cast<uint64_t>(std::floor(posDelta.length() * 100));
            playerStats->addCustomStats(CustomType::fly_one_cm, value);
        } else if (player.isOnGround()) {
            auto const posOffset = Vec3{0, -0.0784, 0};
            auto const value     = static_cast<uint64_t>(std::floor(posDelta.distanceTo(posOffset) * 100));
            if (playerStats->mDistanceCache.isSneaking) {
                playerStats->mDistanceCache.sneak += value;
            } else if (playerStats->mDistanceCache.isSprinting) {
                playerStats->addCustomStats(CustomType::sprint_one_cm, value);
            } else {
                playerStats->addCustomStats(CustomType::walk_one_cm, value);
            }
        } else if (player.onClimbableBlock()) {
            auto const valueY = posDelta.y + 0.0784;
            auto const value  = static_cast<uint64_t>(std::floor(valueY > 0 ? valueY * 100 : 0));
            playerStats->addCustomStats(CustomType::climb_one_cm, value);
        } else if (playerStats->mDistanceCache.isGliding) {
            auto const value = static_cast<uint64_t>(std::floor(posDelta.length() * 100));
            playerStats->addCustomStats(CustomType::aviate_one_cm, value);
        }
    }
    updateGlidingState();
}

void onPlayerJump(Player& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::jump);
}

} // namespace stats::handler
