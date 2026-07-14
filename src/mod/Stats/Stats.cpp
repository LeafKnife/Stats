#include "mod/Stats/Stats.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <parallel_hashmap/phmap.h>

#include <ll/api/i18n/I18n.h>
#include <ll/api/data/CancellableCallback.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>


#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"
#include "mod/MyMod.h"
#include "mod/Stats/Command/RegisterCommand.h"
#include "mod/Stats/StatsFileRepository.h"

using namespace ll::i18n_literals;

namespace stats {
namespace {
using PlayerStatsMap = phmap::flat_hash_map<mce::UUID, std::unique_ptr<PlayerStats>>;

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
std::shared_ptr<ll::data::CancellableCallback> autosaveTask;

constexpr auto AutosaveInterval = std::chrono::minutes{5};
constexpr auto RankCacheTtl     = std::chrono::seconds{30};
constexpr std::size_t MaxFullRankCacheEntries = 16;
constexpr std::size_t MaxRankPageCacheEntries = 64;

using RankCacheClock = std::chrono::steady_clock;

struct RankCacheKey {
    StatsType   type;
    std::string key;
    std::size_t pageIndex;
    std::size_t pageSize;

    bool operator==(RankCacheKey const&) const = default;
};

struct RankCacheKeyHash {
    std::size_t operator()(RankCacheKey const& value) const {
        auto hash = std::hash<std::string>{}(value.key);
        hash ^= std::hash<int>{}(static_cast<int>(value.type)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::size_t>{}(value.pageIndex) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::size_t>{}(value.pageSize) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

template <class Value>
struct CachedRank {
    RankCacheClock::time_point createdAt;
    Value                      value;
};

using FullRankCache =
    phmap::flat_hash_map<RankCacheKey, CachedRank<query::RankData>, RankCacheKeyHash>;
using RankPageCache =
    phmap::flat_hash_map<RankCacheKey, CachedRank<query::StatsPage>, RankCacheKeyHash>;

FullRankCache rankCache;
RankPageCache rankPageCache;

template <class Cache>
void trimRankCache(Cache& cache, std::size_t capacity, RankCacheClock::time_point now) {
    for (auto entry = cache.begin(); entry != cache.end();) {
        if (now - entry->second.createdAt >= RankCacheTtl) {
            cache.erase(entry++);
        } else {
            ++entry;
        }
    }
    if (cache.size() < capacity) return;

    auto oldest = cache.begin();
    for (auto entry = std::next(cache.begin()); entry != cache.end(); ++entry) {
        if (entry->second.createdAt < oldest->second.createdAt) oldest = entry;
    }
    cache.erase(oldest);
}

void clearRankCaches() {
    rankPageCache.clear();
    rankCache.clear();
}

void saveOnlinePlayers() {
    auto const level = ll::service::getLevel();
    if (!level) return;

    auto const currentTick = level->getCurrentTick().tickID;
    for (auto const& [uuid, playerStats] : playerStatsMap) {
        (void)uuid;
        playerStats->checkpoint(currentTick);
        savePlayerStats(*playerStats);
    }
}

void scheduleAutosave() {
    autosaveTask = ll::thread::ServerThreadExecutor::getDefault().executeAfter(
        [] {
            saveOnlinePlayers();
            scheduleAutosave();
        },
        AutosaveInterval
    );
}

void stopAutosave() {
    if (autosaveTask) autosaveTask->cancel();
    autosaveTask.reset();
}
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
        player.getDimensionId().mValue,
        ll::service::getLevel()->getCurrentTick().tickID,
    };
    playerStatsMap.try_emplace(uuid, std::make_unique<PlayerStats>(std::move(session), data));
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

void clearStatsCache() {
    clearRankCaches();
    statsCache.clear();
}

query::RankData getStatsRank(StatsType type, std::string const& key) {
    RankCacheKey const cacheKey{type, key, 0, 0};
    auto const         now = RankCacheClock::now();
    if (auto cached = rankCache.find(cacheKey); cached != rankCache.end()) {
        if (now - cached->second.createdAt < RankCacheTtl) return cached->second.value;
        rankCache.erase(cached);
    }

    query::RankBuilder builder(statsCache.entries().size(), key);

    for (auto const& entry : statsCache.entries()) {
        auto const& data  = entry.second;
        auto const* stats = data.second ? data.second->getMap(type) : nullptr;
        builder.add(data.first.name, stats);
    }
    auto result = std::move(builder).finish();
    trimRankCache(rankCache, MaxFullRankCacheEntries, now);
    rankCache.insert_or_assign(cacheKey, CachedRank<query::RankData>{RankCacheClock::now(), result});
    return result;
}

query::StatsPage getStatsRankPage(
    StatsType          type,
    std::string const& key,
    std::size_t        pageIndex,
    std::size_t        pageSize
) {
    pageSize = std::clamp(pageSize, std::size_t{1}, query::MaxPageSize);
    RankCacheKey const cacheKey{type, key, pageIndex, pageSize};
    auto const         now = RankCacheClock::now();
    if (auto cached = rankPageCache.find(cacheKey); cached != rankPageCache.end()) {
        if (now - cached->second.createdAt < RankCacheTtl) return cached->second.value;
        rankPageCache.erase(cached);
    }

    query::RankBuilder builder(statsCache.entries().size(), key);
    for (auto const& entry : statsCache.entries()) {
        auto const& data  = entry.second;
        auto const* stats = data.second ? data.second->getMap(type) : nullptr;
        builder.add(data.first.name, stats);
    }
    auto result = std::move(builder).finishPage(pageIndex, pageSize);
    trimRankCache(rankPageCache, MaxRankPageCacheEntries, now);
    rankPageCache.insert_or_assign(cacheKey, CachedRank<query::StatsPage>{RankCacheClock::now(), result});
    return result;
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
        scheduleAutosave();
        getLogger().info("plugins.load.success"_tr());
    } else {
        repository::shutdown();
        getLogger().warn("plugins.load.fail"_tr());
    }
}

void unload() {
    stopAutosave();
    saveOnlinePlayers();
    event::removeEvents();
    hook::unhook();
    repository::shutdown();
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
