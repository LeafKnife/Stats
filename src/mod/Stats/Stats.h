#pragma once

#include "mod/Stats/PlayerStats.h"
#include "mod/Stats/StatsJsonCodec.h"
#include "mod/Stats/StatsQuery.h"

#include <ll/api/io/Logger.h>
#include <mc/platform/UUID.h>
#include <memory>
#include <vector>

namespace stats {
typedef std::pair<PlayerInfo, std::shared_ptr<StatsData>> StatsCacheData;

ll::io::Logger&       getLogger();
PlayerStats*          findPlayerStats(mce::UUID const& uuid);
void                  addPlayerStats(std::shared_ptr<PlayerStats> playerStats);
void                  removePlayerStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStatsByName(std::string const& name);
void                  upsertStatsCache(StatsCacheData data);
void                  clearStatsCache();
query::RankData       getStatsRank(StatsType type, std::string const& key);

bool loadStatsCache();
void load();
void unload();
void printLogo();
void exportRemoteCall();
} // namespace stats
