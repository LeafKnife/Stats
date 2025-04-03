#include "lk/Entry.h"

#include "lk/stats/Stats.h"
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/RegisterHelper.h>

namespace lk {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() {
    getSelf().getLogger().debug("Loading...");
    auto res = ll::i18n::getInstance().load(getSelf().getLangDir());
    stats::printLogo();
    // Code for loading the mod goes here.
    return true;
}

bool Entry::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    stats::load();
    return true;
}

bool Entry::disable() {
    getSelf().getLogger().debug("Disabling...");
    stats::unload();
    // Code for disabling the mod goes here.
    return true;
}

} // namespace lk

LL_REGISTER_MOD(lk::Entry, lk::Entry::getInstance());
