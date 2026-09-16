#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>

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
namespace {

// A player normally moves well below this in a single auth-input packet. Larger
// deltas are teleports or invalid client input and must not become statistics.
constexpr float MaxDistancePerAuthInput = 10.0f;

std::optional<uint64_t> toCentimeters(float distance) {
    if (!std::isfinite(distance) || distance <= 0.0f || distance > MaxDistancePerAuthInput) return std::nullopt;
    return static_cast<uint64_t>(std::floor(static_cast<double>(distance) * 100.0));
}

uint64_t saturatingAdd(uint64_t current, uint64_t value) {
    return value > std::numeric_limits<uint64_t>::max() - current ? std::numeric_limits<uint64_t>::max()
                                                                  : current + value;
}

} // namespace

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

void onPlayerStartRiding(Player& player) {
    auto* playerStats = findPlayerStats(player.getUuid());
    if (!playerStats) return;
    playerStats->mDistanceCache.ride = 0;
    playerStats->mLastPos            = player.getPosition();
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
    if (player.getVehicle() != nullptr) {
        if (auto const value = toCentimeters(position.distanceTo(playerStats->mLastPos))) {
            playerStats->mDistanceCache.ride = saturatingAdd(playerStats->mDistanceCache.ride, *value);
        }
        playerStats->mLastPos = position;
    } else {
        // Some dismount paths may not emit the stop-riding callback.
        playerStats->mDistanceCache.ride = 0;

        if (player.isInWaterOrRain()) {
            if (auto const value = toCentimeters(posDelta.length()); value && player.isSwimming()) {
                playerStats->addCustomStats(CustomType::swim_one_cm, *value);
            } else if (value && player.isImmersedInWater()) {
                playerStats->addCustomStats(CustomType::walk_on_water_one_cm, *value);
            } else if (value) {
                playerStats->addCustomStats(CustomType::walk_under_water_one_cm, *value);
            }
        } else if (player.isFlying()) {
            if (auto const value = toCentimeters(posDelta.length())) {
                playerStats->addCustomStats(CustomType::fly_one_cm, *value);
            }
        } else if (player.isOnGround()) {
            auto const posOffset = Vec3{0, -0.0784, 0};
            if (auto const value = toCentimeters(posDelta.distanceTo(posOffset))) {
                if (playerStats->mDistanceCache.isSneaking) {
                    playerStats->mDistanceCache.sneak = saturatingAdd(playerStats->mDistanceCache.sneak, *value);
                } else if (playerStats->mDistanceCache.isSprinting) {
                    playerStats->addCustomStats(CustomType::sprint_one_cm, *value);
                } else {
                    playerStats->addCustomStats(CustomType::walk_one_cm, *value);
                }
            }
        } else if (player.onClimbableBlock()) {
            auto const valueY = posDelta.y + 0.0784;
            if (auto const value = toCentimeters(valueY)) {
                playerStats->addCustomStats(CustomType::climb_one_cm, *value);
            }
        } else if (playerStats->mDistanceCache.isGliding) {
            if (auto const value = toCentimeters(posDelta.length())) {
                playerStats->addCustomStats(CustomType::aviate_one_cm, *value);
            }
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
