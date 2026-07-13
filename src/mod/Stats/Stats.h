#pragma once

#include "mod/Stats/PlayerStats.h"
#include "mod/Stats/StatsQuery.h"

#include <ll/api/io/Logger.h>
#include <mc/platform/UUID.h>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace stats {
struct PlayerInfo {
    std::string uuid;
    std::string xuid;
    std::string name;
};
typedef std::pair<PlayerInfo, std::shared_ptr<StatsData>>           StatsCacheData;

ll::io::Logger&       getLogger();
PlayerStats*          findPlayerStats(mce::UUID const& uuid);
void                  addPlayerStats(std::shared_ptr<PlayerStats> playerStats);
void                  removePlayerStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStats(mce::UUID const& uuid);
StatsCacheData const* findCachedStatsByName(std::string const& name);
void                  addStatsCache(StatsCacheData data);
void                  clearStatsCache();
StatsCacheData        parseStatsData(std::string const& data);
query::RankData       getStatsRank(StatsType type, std::string const& key);

std::filesystem::path getStatsPath();

bool loadStatsCache();
void load();
void unload();
void printLogo();
void exportRemoteCall();
} // namespace stats
