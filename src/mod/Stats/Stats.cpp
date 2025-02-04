#include "mod/Stats/Stats.h"

#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"

namespace stats {
namespace {
PlayerStatsMap playerStatsMap;
}

PlayerStatsMap& getPlayerStatsMap(){
    return playerStatsMap;
}

void load(){
    event::listenEvents();
    hook::hook();
}

void unload(){
    event::removeEvents();
    hook::unhook();
}
} // namespace Stats