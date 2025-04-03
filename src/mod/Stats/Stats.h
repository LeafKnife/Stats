#pragma once

#include "mod/Stats/PlayerStats.h"

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
typedef std::map<mce::UUID, std::shared_ptr<PlayerStats>> PlayerStatsMap;
typedef std::pair<PlayerInfo, std::shared_ptr<StatsData>> StatsCacheData;
typedef std::vector<StatsCacheData>                       StatsCache;

ll::io::Logger& getLogger();
PlayerStatsMap& getPlayerStatsMap();
StatsCache&     getStatsCache();
StatsCacheData  parseStatsData(std::string const& data);

std::filesystem::path getStatsPath();

bool loadStatsCache();
void load();
void unload();
void printLogo();
void exportRemoteCall();
} // namespace stats