#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>

#include "mod/Stats/Stats.h"

namespace stats::handler {

void onPlayerJoin(Player& player) {
    if (player.isSimulatedPlayer()) return;
    addPlayerStats(player);
}

void onPlayerDisconnect(ServerPlayer& player) {
    auto  uuid        = player.getUuid();
    auto* playerStats = findPlayerStats(uuid);
    if (!playerStats) return;

    playerStats->checkpoint(ll::service::getLevel()->getCurrentTick().tickID);
    playerStats->addCustomStats(CustomType::leave_game);
    savePlayerStats(*playerStats);
    removePlayerStats(uuid);
}

} // namespace stats::handler
