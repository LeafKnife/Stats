#include "mod/Stats/StatsData.h"

namespace stats {
StatsDataMap* StatsData::getMap(StatsType type) {
    switch (type) {
    case StatsType::custom:
        return &custom;
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
    }
    return nullptr;
}
} // namespace stats