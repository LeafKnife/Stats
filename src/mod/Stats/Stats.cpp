#include "mod/Stats/Stats.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <parallel_hashmap/phmap.h>

#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/PropertiesSettings.h>


#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"
#include "mod/MyMod.h"
#include "mod/Stats/Command/RegisterCommand.h"

using namespace ll::i18n_literals;

namespace stats {
namespace {
using PlayerStatsMap = phmap::flat_hash_map<mce::UUID, std::shared_ptr<PlayerStats>>;

class StatsCacheStore {
public:
    using UuidIndex = phmap::node_hash_map<mce::UUID, StatsCacheData>;

    StatsCacheData const* find(mce::UUID const& uuid) const {
        auto const cached = mByUuid.find(uuid);
        return cached == mByUuid.end() ? nullptr : &cached->second;
    }

    StatsCacheData const* findByName(std::string const& name) const {
        auto const cached = mUuidByName.find(name);
        return cached == mUuidByName.end() ? nullptr : find(cached->second);
    }

    void upsert(StatsCacheData data) {
        auto const uuid   = mce::UUID(data.first.uuid);
        auto       cached = mByUuid.find(uuid);
        if (cached != mByUuid.end()) {
            auto const oldName = cached->second.first.name;
            if (auto const old = mUuidByName.find(oldName); old != mUuidByName.end() && old->second == uuid) {
                mUuidByName.erase(old);
            }
            cached->second = std::move(data);
            mUuidByName.insert_or_assign(cached->second.first.name, uuid);
            return;
        }

        auto const inserted = mByUuid.try_emplace(uuid, std::move(data)).first;
        mUuidByName.insert_or_assign(inserted->second.first.name, uuid);
    }

    void clear() {
        mUuidByName.clear();
        mByUuid.clear();
    }

    void reserve(std::size_t size) {
        mByUuid.reserve(size);
        mUuidByName.reserve(size);
    }

    UuidIndex const& entries() const { return mByUuid; }

private:
    UuidIndex mByUuid;
    // Player names are secondary keys; the most recently upserted UUID owns a collision.
    phmap::flat_hash_map<std::string, mce::UUID> mUuidByName;
};

PlayerStatsMap  playerStatsMap;
StatsCacheStore statsCache;
std::string     levelName;

StatsCacheData decodeCachedStats(std::string_view source) {
    auto decoded = decodeStatsJson(source);
    return std::make_pair(std::move(decoded.info), std::make_shared<StatsData>(std::move(decoded.data)));
}
} // namespace

ll::io::Logger& getLogger() { return lk::MyMod::getInstance().getSelf().getLogger(); }
PlayerStats*    findPlayerStats(mce::UUID const& uuid) {
    auto const player = playerStatsMap.find(uuid);
    return player == playerStatsMap.end() ? nullptr : player->second.get();
}

void addPlayerStats(std::shared_ptr<PlayerStats> playerStats) {
    auto const uuid = playerStats->getUuid();
    playerStatsMap.try_emplace(uuid, std::move(playerStats));
}

void removePlayerStats(mce::UUID const& uuid) { playerStatsMap.erase(uuid); }

StatsCacheData const* findCachedStats(mce::UUID const& uuid) { return statsCache.find(uuid); }

StatsCacheData const* findCachedStatsByName(std::string const& name) { return statsCache.findByName(name); }

void upsertStatsCache(StatsCacheData data) { statsCache.upsert(std::move(data)); }

void clearStatsCache() { statsCache.clear(); }

query::RankData getStatsRank(StatsType type, std::string const& key) {
    std::vector<query::RankEntryView> entries;
    entries.reserve(statsCache.entries().size());

    for (auto const& entry : statsCache.entries()) {
        auto const& data  = entry.second;
        auto const* stats = data.second ? data.second->getMap(type) : nullptr;
        entries.push_back({data.first.name, stats});
    }
    return query::buildRank(entries, key);
}

inline std::optional<std::string> getLevelName() {
    if (ll::service::getPropertiesSettings().has_value()) {
        return ll::service::getPropertiesSettings()->mLevelName;
    } else {
        std::ifstream fin("server.properties");
        std::string   buf;
        while (getline(fin, buf)) {
            if (buf.find("level-name=") != std::string::npos) {
                if (buf.back() == '\n') buf.pop_back();
                if (buf.back() == '\r') buf.pop_back();
                return buf.substr(11);
            }
        }
    }
    return std::nullopt;
}
std::filesystem::path getStatsPath() {
    if (!levelName.empty()) {
        return ll::file_utils::u8path("./worlds/" + levelName + "/stats");
    } else {
        auto name = getLevelName();
        if (name.has_value()) {
            levelName = name.value();
        }
        return ll::file_utils::u8path("./worlds/" + levelName + "/stats");
    }
}

bool loadStatsCache() {
    clearStatsCache();
    auto        oldPath      = ll::file_utils::u8path("./stats");
    auto        newPath      = getStatsPath();
    std::string extension    = ".json";
    auto        existOldPath = std::filesystem::exists(oldPath);
    auto        existPath    = std::filesystem::exists(newPath);
    if (!existPath) {
        if (existOldPath) {
            getLogger().warn("log.info.ExistOldPath"_tr());
            try {
                std::filesystem::rename(oldPath, newPath);
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("log.warn.moveStats.fail"_tr());
                return false;
            }
        } else {
            getLogger().warn("log.info.CreateStatsPath"_tr());
            try {
                std::filesystem::create_directory(newPath);
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("log.warn.CreatePath.fail"_tr());
                return false;
            }
        }
    }
    std::vector<std::filesystem::path> statsFiles;
    for (auto const& entry : std::filesystem::directory_iterator(newPath)) {
        if (entry.path().extension() == extension) statsFiles.push_back(entry.path());
    }

    statsCache.reserve(statsFiles.size());
    for (auto const& path : statsFiles) {
        auto rawData = ll::file_utils::readFile(path);
        if (!rawData.has_value()) {
            getLogger().warn("data.parse.fail"_tr(path.filename()));
            continue;
        }
        try {
            upsertStatsCache(decodeCachedStats(*rawData));
        } catch (std::exception& excep) {
            getLogger().error(excep.what());
            getLogger().warn("data.parse.fail"_tr(path.filename()));
        }
    }
    return true;
}

void load() {
    if (loadStatsCache()) {
        event::listenEvents();
        hook::hook();
        command::registerCommand();
        exportRemoteCall();
        getLogger().info("plugins.load.success"_tr());
    } else {
        getLogger().warn("plugins.load.fail"_tr());
    }
}

void unload() {
    event::removeEvents();
    hook::unhook();
}

void printLogo() {
    auto& logger = getLogger();
    logger.info(R"(                                                                         )");
    logger.info(R"(     ██╗     ██╗  ██╗     ███████╗████████╗ █████╗ ████████╗███████╗     )");
    logger.info(R"(     ██║     ██║ ██╔╝     ██╔════╝╚══██╔══╝██╔══██╗╚══██╔══╝██╔════╝     )");
    logger.info(R"(     ██║     █████╔╝█████╗███████╗   ██║   ███████║   ██║   ███████╗     )");
    logger.info(R"(     ██║     ██╔═██╗╚════╝╚════██║   ██║   ██╔══██║   ██║   ╚════██║     )");
    logger.info(R"(     ███████╗██║  ██╗     ███████║   ██║   ██║  ██║   ██║   ███████║     )");
    logger.info(R"(     ╚══════╝╚═╝  ╚═╝     ╚══════╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚══════╝     )");
    logger.info(R"(                                                                         )");
    /// logger.info(R"(     ----------------   LeafKnife Statistics Plugin  ---------------     )");
    logger.info("Repository: https://github.com/LeafKnife/Stats");
}
} // namespace stats
