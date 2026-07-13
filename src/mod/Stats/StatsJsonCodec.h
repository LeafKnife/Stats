#pragma once

#include "mod/Stats/StatsData.h"
#include "mod/Stats/StatsQuery.h"
#include "mod/Stats/StatsRecord.h"

#include <string>
#include <string_view>

namespace stats {

struct DecodedStats {
    PlayerInfo info;
    StatsData  data;
};

std::string  encodeStatsJson(PlayerInfo const& info, StatsData const& data);
DecodedStats decodeStatsJson(std::string_view json);
std::string encodeStatsPageJson(query::StatsPage const& page, std::string_view labelField);

} // namespace stats
