#include "mod/Stats/Stats.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <parallel_hashmap/phmap.h>

#include <ll/api/i18n/I18n.h>
#include <mc/world/actor/player/Player.h>


#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"
#include "mod/MyMod.h"
#include "mod/Stats/Command/RegisterCommand.h"
#include "mod/Stats/StatsFileRepository.h"

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
} // namespace

ll::io::Logger& getLogger() { return lk::MyMod::getInstance().getSelf().getLogger(); }
PlayerStats*    findPlayerStats(mce::UUID const& uuid) {
    auto const player = playerStatsMap.find(uuid);
    return player == playerStatsMap.end() ? nullptr : player->second.get();
}

void addPlayerStats(Player const& player) {
    auto const  uuid             = player.getUuid();
    auto const* cached           = findCachedStats(uuid);
    bool const  needsInitialSave = !cached || !cached->second;
    auto        data             = needsInitialSave ? std::make_shared<StatsData>() : cached->second;

    PlayerInfo info{uuid.asString(), player.getXuid(), player.getRealName()};
    if (needsInitialSave) getLogger().debug("log.info.createData"_tr(info.name));
    upsertStatsCache(std::make_pair(info, data));
    PlayerSessionInit session{
        uuid,
        info.xuid,
        info.name,
        player.getPosition(),
        player.getDimensionId().id,
    };
    playerStatsMap.try_emplace(uuid, std::make_shared<PlayerStats>(std::move(session), data));
    if (needsInitialSave) repository::save(info, *data);
}

bool savePlayerStats(PlayerStats const& playerStats) {
    auto const info = playerStats.getInfo();
    getLogger().debug("log.info.savaData"_tr(info.name));
    return repository::save(info, playerStats.getData());
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

bool loadStatsCache() {
    clearStatsCache();
    if (!repository::initialize()) return false;

    std::vector<DecodedStats> records;
    if (!repository::loadAll(records)) return false;
    statsCache.reserve(records.size());
    for (auto& record : records) {
        upsertStatsCache(std::make_pair(std::move(record.info), std::make_shared<StatsData>(std::move(record.data))));
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
