#include "mod/Stats/Stats.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
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

    void add(StatsCacheData data) {
        auto const uuid   = mce::UUID(data.first.uuid);
        auto       cached = mByUuid.find(uuid);
        if (cached != mByUuid.end()) {
            auto const oldName = cached->second.first.name;
            if (auto const old = mUuidByName.find(oldName);
                old != mUuidByName.end() && old->second == uuid) {
                mUuidByName.erase(old);
            }
            cached->second = std::move(data);
            mUuidByName.try_emplace(cached->second.first.name, uuid);
            return;
        }

        auto const inserted = mByUuid.try_emplace(uuid, std::move(data)).first;
        mUuidByName.try_emplace(inserted->second.first.name, uuid);
    }

    void clear() {
        mUuidByName.clear();
        mByUuid.clear();
    }

    UuidIndex const& entries() const { return mByUuid; }

private:
    UuidIndex                                    mByUuid;
    phmap::flat_hash_map<std::string, mce::UUID> mUuidByName;
};

PlayerStatsMap  playerStatsMap;
StatsCacheStore statsCache;
std::string     levelName;
} // namespace

ll::io::Logger& getLogger() { return lk::MyMod::getInstance().getSelf().getLogger(); }
PlayerStatsMap& getPlayerStatsMap() { return playerStatsMap; }
PlayerStats*    findPlayerStats(mce::UUID const& uuid) {
    auto const player = playerStatsMap.find(uuid);
    return player == playerStatsMap.end() ? nullptr : player->second.get();
}
StatsCacheData const* findCachedStats(mce::UUID const& uuid) {
    return statsCache.find(uuid);
}

StatsCacheData const* findCachedStatsByName(std::string const& name) {
    return statsCache.findByName(name);
}

void addStatsCache(StatsCacheData data) { statsCache.add(std::move(data)); }

void clearStatsCache() { statsCache.clear(); }

StatsCacheData parseStatsData(const std::string& data) {
    auto j       = nlohmann::json::parse(data);
    auto tmpData = std::make_shared<StatsData>();
    auto tmpInfo = PlayerInfo();
    tmpInfo.name = j["playerInfo"]["name"];
    tmpInfo.uuid = j["playerInfo"]["uuid"];
    tmpInfo.xuid = j["playerInfo"]["xuid"];
    tmpData->custom.load(j["minecraft:custom"].get<StatsDataMap>());
    tmpData->mined     = j["minecraft:mined"];
    tmpData->broken    = j["minecraft:broken"];
    tmpData->crafted   = j["minecraft:crafted"];
    tmpData->used      = j["minecraft:used"];
    tmpData->picked_up = j["minecraft:picked_up"];
    tmpData->dropped   = j["minecraft:dropped"];
    tmpData->killed    = j["minecraft:killed"];
    tmpData->killed_by = j["minecraft:killed_by"];
    StatsCacheData r   = std::make_pair(tmpInfo, tmpData);
    return r;
}

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
    for (const auto& entry : std::filesystem::directory_iterator(newPath)) {
        // 检查文件是否为所需后缀
        if (entry.path().extension() == extension) {
            auto rawData = ll::file_utils::readFile(entry.path());
            if (!rawData.has_value()) {
                getLogger().warn("data.parse.fail"_tr(entry.path().filename()));
                continue;
            }
            try {
                addStatsCache(parseStatsData(*rawData));
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("data.parse.fail"_tr(entry.path().filename()));
            }
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
