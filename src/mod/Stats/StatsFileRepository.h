#pragma once

#include "mod/Stats/StatsJsonCodec.h"

#include <vector>

namespace stats::repository {

bool initialize();
bool loadAll(std::vector<DecodedStats>& records);
bool save(PlayerInfo const& info, StatsData const& data);

} // namespace stats::repository
