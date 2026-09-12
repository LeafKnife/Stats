#include "mod/MyMod.h"

#include "mod/Stats/Stats.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/RegisterHelper.h>
#include <nlohmann/json.hpp>

namespace {

void loadTranslationNode(
    nlohmann::json const& data,
    std::string const&    locale,
    std::string const&    prefix,
    ll::i18n::I18n&       i18n
) {
    for (auto const& [name, value] : data.items()) {
        auto const key = prefix.empty() ? name : prefix + "." + name;
        if (value.is_string()) {
            i18n.set(locale, key, value.get<std::string>());
        } else if (value.is_object()) {
            loadTranslationNode(value, locale, key, i18n);
        }
    }
}

bool loadTranslations(std::filesystem::path const& langDir, ll::i18n::I18n& i18n) {
    std::error_code error;
    for (std::filesystem::directory_iterator it(langDir, error), end; !error && it != end; it.increment(error)) {
        if (!it->is_regular_file(error) || it->path().extension() != ".json") continue;

        std::ifstream input(it->path());
        auto          data = nlohmann::json::parse(input, nullptr, false);
        if (!input || data.is_discarded() || !data.is_object()) return false;

        loadTranslationNode(data, it->path().stem().string(), {}, i18n);
    }
    return !error;
}

} // namespace

namespace lk {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    if (!loadTranslations(getSelf().getLangDir(), ll::i18n::getInstance())) {
        getSelf().getLogger().error("Failed to load translations");
    }
    stats::printLogo();
    // Code for loading the mod goes here.
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    stats::load();
    return true;
}

bool MyMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    stats::unload();
    // Code for disabling the mod goes here.
    return true;
}

} // namespace lk

LL_REGISTER_MOD(lk::MyMod, lk::MyMod::getInstance());
