#include "mod/Stats/Stats.h"

#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"
#include "mod/Stats/Command/RegisterCommand.h"


namespace stats {
namespace {
PlayerStatsMap playerStatsMap;
}

PlayerStatsMap& getPlayerStatsMap() { return playerStatsMap; }

void load() {
    event::listenEvents();
    hook::hook();
    command::registerCommand();
}

void unload() {
    event::removeEvents();
    hook::unhook();
}
} // namespace stats