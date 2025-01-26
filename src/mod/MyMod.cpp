#include "mod/MyMod.h"
#include "mod/Stats/Events.h"
#include <memory>

#include "ll/api/mod/RegisterHelper.h"

namespace Stats {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    Stats::event::listenEvents();
    Stats::event::hook::hook();
    return true;
}

bool MyMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    Stats::event::removeEvents();
    Stats::event::hook::unhook();
    // Code for disabling the mod goes here.
    return true;
}

} // namespace my_mod

LL_REGISTER_MOD(Stats::MyMod, Stats::MyMod::getInstance());
