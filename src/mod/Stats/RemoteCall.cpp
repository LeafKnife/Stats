#include "mc/platform/UUID.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsData.h"

#include <RemoteCallAPI.h>
#include <numeric>
#include <unordered_map>
#include <utility>

namespace stats {
inline uint64_t getStatsDataMapValue(StatsDataMap const& map, std::string type) {
    uint64_t value;
    if (type.empty()) {
        value = std::accumulate(
            map.begin(),
            map.end(),
            0,
            [](int total, const std::pair<std::string, unsigned long long>& p) { return total + p.second; }
        );
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

inline void
getRankData(std::vector<std::pair<std::string, unsigned long long>>& data, StatsType statsType, std::string type) {
    auto& cache = getStatsCache();
    for (auto it : cache) {
        auto name  = it.first.name;
        auto map   = it.second->getMap(statsType);
        auto value = getStatsDataMapValue(*map, type);
        data.push_back(std::make_pair(name, value));
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
            for (auto& data : cache) {
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
            for (auto& a : r) {
                data.emplace(a.first, a.second);
            }
            return data;
        }
    );
}
} // namespace stats