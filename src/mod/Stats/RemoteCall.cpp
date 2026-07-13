#include "mc/platform/UUID.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsData.h"

#include <RemoteCallAPI.h>
#include <unordered_map>
#include <utility>

namespace stats {
void exportRemoteCall() {
    std::string const RC_NAMESPACE = "LK-Stats";
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getPlayerStats",
        [](std::string uuidStr, int type) -> std::unordered_map<std::string, unsigned long long> {
            mce::UUID uuid(uuidStr);
            auto      r = std::unordered_map<std::string, unsigned long long>();
            if (type < 1 || type > 9) return r;
            auto const* cached = findCachedStats(uuid);
            if (cached && cached->second) {
                return *cached->second->getMap((StatsType)type);
            }
            return r;
        }
    );
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getRankStats",
        [](int type, std::string key = "") -> std::unordered_map<std::string, unsigned long long> {
            auto r = query::RankData{};
            if (type < 1 || type > 9) {
            } else if (type == 1 && key.empty()) {
            } else {
                r = getStatsRank((StatsType)type, key);
            }
            auto data = std::unordered_map<std::string, unsigned long long>();
            data.reserve(r.size());
            for (auto const& a : r) {
                data.emplace(a.first, a.second);
            }
            return data;
        }
    );
}
} // namespace stats
