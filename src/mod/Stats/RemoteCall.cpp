#include "mc/platform/UUID.h"
#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsData.h"
#include "mod/Stats/StatsJsonCodec.h"

#include <RemoteCallAPI.h>
#include <unordered_map>
#include <utility>

namespace stats {
namespace {

std::size_t pageIndexFromRemote(int page) { return page <= 1 ? 0 : static_cast<std::size_t>(page - 1); }

std::size_t pageSizeFromRemote(int pageSize) {
    return pageSize <= 0 ? query::DefaultPageSize : static_cast<std::size_t>(pageSize);
}

query::StatsPage getPlayerStatsPage(mce::UUID const& uuid, StatsType type, int page, int pageSize) {
    query::StatsEntries entries;
    auto const*         cached = findCachedStats(uuid);
    auto const*         values = cached && cached->second ? cached->second->getMap(type) : nullptr;
    if (values) {
        entries.reserve(values->size());
        for (auto const& entry : *values) entries.push_back(entry);
        query::sortStatsEntries(entries, type == StatsType::custom);
    }
    return query::paginate(std::move(entries), pageIndexFromRemote(page), pageSizeFromRemote(pageSize));
}

} // namespace

void exportRemoteCall() {
    std::string const RC_NAMESPACE = "LK-Stats";
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getPlayerStats",
        [](std::string uuidStr, int type) -> std::unordered_map<std::string, unsigned long long> {
            mce::UUID  uuid(uuidStr);
            auto       r         = std::unordered_map<std::string, unsigned long long>();
            auto const statsType = static_cast<StatsType>(type);
            if (!isValidStatsType(statsType)) return r;
            auto const* cached = findCachedStats(uuid);
            if (cached && cached->second) {
                return *cached->second->getMap(statsType);
            }
            return r;
        }
    );
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getRankStats",
        [](int type, std::string key = "") -> std::unordered_map<std::string, unsigned long long> {
            auto const statsType = static_cast<StatsType>(type);
            auto       r         = query::RankData{};
            if (isValidStatsType(statsType) && (statsType != StatsType::custom || !key.empty())) {
                r = getStatsRank(statsType, key);
            }
            auto data = std::unordered_map<std::string, unsigned long long>();
            data.reserve(r.size());
            for (auto const& a : r) {
                data.emplace(a.first, a.second);
            }
            return data;
        }
    );
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getPlayerStatsPage",
        [](std::string uuidStr, int type, int page, int pageSize) -> std::string {
            auto const statsType = static_cast<StatsType>(type);
            if (!isValidStatsType(statsType)) {
                return encodeStatsPageJson(query::paginate({}, 0, pageSizeFromRemote(pageSize)), "key");
            }
            return encodeStatsPageJson(
                getPlayerStatsPage(mce::UUID(uuidStr), statsType, page, pageSize),
                "key"
            );
        }
    );
    RemoteCall::exportAs(
        RC_NAMESPACE,
        "getRankStatsPage",
        [](int type, std::string key, int page, int pageSize) -> std::string {
            auto const statsType = static_cast<StatsType>(type);
            query::StatsPage rankPage = query::paginate({}, 0, pageSizeFromRemote(pageSize));
            if (isValidStatsType(statsType) && (statsType != StatsType::custom || !key.empty())) {
                rankPage = getStatsRankPage(
                    statsType,
                    key,
                    pageIndexFromRemote(page),
                    pageSizeFromRemote(pageSize)
                );
            }
            return encodeStatsPageJson(rankPage, "player");
        }
    );
}
} // namespace stats
