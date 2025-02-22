#pragma once

#include <mc/platform/UUID.h>
#include "mod/Stats/PlayerStats.h"
#include "mod/Stats/StatsType.h"

#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace stats {
typedef std::map<mce::UUID, std::shared_ptr<PlayerStats>>                  PlayerStatsMap;
typedef std::pair<PlayerInfo, std::shared_ptr<StatsData>> StatsCacheData;
typedef std::vector<StatsCacheData>                                        StatsCache;

PlayerStatsMap& getPlayerStatsMap();
StatsCache&     getStatsCache();
StatsCacheData parseStatsData(std::string const& data);
void            loadStatsCache();
void            load();
void            unload();
} // namespace stats