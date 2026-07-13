#include "mc/platform/UUID.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsData.h"

#include <RemoteCallAPI.h>
#include <numeric>
#include <unordered_map>
#include <utility>

namespace stats {
inline uint64_t getStatsDataMapValue(StatsDataMap const& map, std::string const& type) {
    uint64_t value;
    if (type.empty()) {
        value = std::accumulate(map.begin(), map.end(), uint64_t{0}, [](uint64_t total, auto const& pair) {
            return total + pair.second;
        });
    } else {
        auto mapValue = map.find(type);
        if (mapValue != map.end()) {
            value = mapValue->second;
        } else {
            value = 0;
        }
    }
    return value;
}

inline void getRankData(
    std::vector<std::pair<std::string, unsigned long long>>& data,
    StatsType                                                statsType,
    std::string const&                                       type
) {
    auto& cache = getStatsCache();
    data.reserve(cache.size());
    for (auto const& it : cache) {
        auto const* map = it.second->getMap(statsType);
        if (!map) continue;
        data.emplace_back(it.first.name, getStatsDataMapValue(*map, type));
    };
    std::sort(
        data.begin(),
        data.end(),
        [](const std::pair<std::string, unsigned long long>& a, const std::pair<std::string, unsigned long long>& b) {
            return a.second > b.second;
        }
    );
}
void exportRemoteCall() {
    std::string const RC_NAMESPACE = "LK-Stats";
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getPlayerStats",
        [](std::string uuidStr, int type) -> std::unordered_map<std::string, unsigned long long> {
            mce::UUID uuid(uuidStr);
            auto&     cache = getStatsCache();
            auto      r     = std::unordered_map<std::string, unsigned long long>();
            if (type < 1 || type > 9) return r;
            for (auto const& data : cache) {
                if (mce::UUID(data.first.uuid) == uuid) {
                    return *data.second->getMap((StatsType)type);
                }
            }
            return r;
        }
    );
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getRankStats",
        [](int type, std::string key = "") -> std::unordered_map<std::string, unsigned long long> {
            auto r = std::vector<std::pair<std::string, unsigned long long>>();
            if (type < 1 || type > 9) {
            } else if (type == 1 && key.empty()) {
            } else {
                getRankData(r, (StatsType)type, key);
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
