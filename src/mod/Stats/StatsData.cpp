#include "mod/Stats/StatsData.h"

namespace stats {

void CustomStatsData::load(StatsDataMap const& values) {
    mValues.fill(0);
    mPresent.reset();
    mUnknown = values;

    for (std::size_t index = 0; index < CustomTypeMap.size(); ++index) {
        auto const value = mUnknown.find(CustomTypeMap[index].second);
        if (value == mUnknown.end()) continue;
        mValues[index] = value->second;
        mPresent.set(index);
        mUnknown.erase(value);
    }
    mSnapshotDirty = true;
}

void CustomStatsData::add(CustomType type, uint64_t value) {
    auto const index   = static_cast<std::size_t>(type);
    mValues.at(index) += value;
    mPresent.set(index);
    mSnapshotDirty = true;
}

void CustomStatsData::add(std::string const& key, uint64_t value) {
    for (std::size_t index = 0; index < CustomTypeMap.size(); ++index) {
        if (CustomTypeMap[index].second != key) continue;
        return add(CustomTypeMap[index].first, value);
    }
    mUnknown[key]  += value;
    mSnapshotDirty  = true;
}

void CustomStatsData::set(CustomType type, uint64_t value) {
    auto const index  = static_cast<std::size_t>(type);
    mValues.at(index) = value;
    mPresent.set(index);
    mSnapshotDirty = true;
}

StatsDataMap const& CustomStatsData::asMap() const {
    if (!mSnapshotDirty) return mSnapshot;

    mSnapshot = mUnknown;
    mSnapshot.reserve(mSnapshot.size() + mPresent.count());
    for (std::size_t index = 0; index < CustomTypeMap.size(); ++index) {
        if (!mPresent.test(index)) continue;
        mSnapshot[CustomTypeMap[index].second] = mValues[index];
    }
    mSnapshotDirty = false;
    return mSnapshot;
}

void StatsData::add(StatsType type, std::string const& key, uint64_t value) {
    switch (type) {
    case StatsType::custom:
        return custom.add(key, value);
    case StatsType::mined:
        mined[key] += value;
        return;
    case StatsType::broken:
        broken[key] += value;
        return;
    case StatsType::crafted:
        crafted[key] += value;
        return;
    case StatsType::used:
        used[key] += value;
        return;
    case StatsType::picked_up:
        picked_up[key] += value;
        return;
    case StatsType::dropped:
        dropped[key] += value;
        return;
    case StatsType::killed:
        killed[key] += value;
        return;
    case StatsType::killed_by:
        killed_by[key] += value;
        return;
    case StatsType::count:
        return;
    }
}

StatsDataMap const* StatsData::getMap(StatsType type) const {
    switch (type) {
    case StatsType::custom:
        return &custom.asMap();
    case StatsType::mined:
        return &mined;
    case StatsType::broken:
        return &broken;
    case StatsType::crafted:
        return &crafted;
    case StatsType::used:
        return &used;
    case StatsType::picked_up:
        return &picked_up;
    case StatsType::dropped:
        return &dropped;
    case StatsType::killed:
        return &killed;
    case StatsType::killed_by:
        return &killed_by;
    case StatsType::count:
        return nullptr;
    }
    return nullptr;
}
} // namespace stats
