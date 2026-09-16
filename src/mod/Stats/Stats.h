#pragma once

#include "mod/Stats/PlayerStats.h"
#include "mod/Stats/StatsQuery.h"
#include "mod/Stats/StatsRecord.h"

#include <ll/api/io/Logger.h>
#include <mc/platform/UUID.h>
#include <cstddef>
#include <memory>
#include <vector>

class Player;

namespace stats {
typedef std::pair<PlayerInfo, std::shared_ptr<StatsData>> StatsCacheData;

ll::io::Logger&       getLogger();
PlayerStats*          findPlayerStats(mce::UUID const& uuid);
void                  addPlayerStats(Player const& player);
bool                  savePlayerStats(PlayerStats const& playerStats);
void                  removePlayerStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStatsByName(std::string const& name);
void                  upsertStatsCache(StatsCacheData data);
void                  clearStatsCache();
query::RankData       getStatsRank(StatsType type, std::string const& key);
query::StatsPage      getStatsRankPage(
    StatsType         type,
    std::string const& key,
    std::size_t       pageIndex,
    std::size_t       pageSize = query::DefaultPageSize
);

bool loadStatsCache();
void load();
void unload();
void printLogo();
void exportRemoteCall();
} // namespace stats
