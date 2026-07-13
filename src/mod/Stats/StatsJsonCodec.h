#pragma once

#include "mod/Stats/StatsData.h"

#include <string>
#include <string_view>

namespace stats {

struct PlayerInfo {
    std::string uuid;
    std::string xuid;
    std::string name;
};

struct DecodedStats {
    PlayerInfo info;
    StatsData  data;
};

std::string  encodeStatsJson(PlayerInfo const& info, StatsData const& data);
DecodedStats decodeStatsJson(std::string_view json);

} // namespace stats
