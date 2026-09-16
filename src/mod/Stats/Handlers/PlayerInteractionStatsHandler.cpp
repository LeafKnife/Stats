#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h>

#include "mod/Stats/Stats.h"

namespace stats::handler {

void onPlayerStartSleep(Player* player) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::sleep_in_bed);
}

void onPlayerOpenTrading(Player* player) {
    auto  uuid        = player->getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::talked_to_villager);
}

void onPlayerBreedAnimal(mce::UUID uuid) {
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::animals_bred);
}

void onPlayerFishCaught(mce::UUID uuid) {
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;
    playerStats->addCustomStats(CustomType::fish_caught);
}

} // namespace stats::handler
