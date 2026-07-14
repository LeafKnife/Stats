#include "mod/Stats/StatsCustom.h"

#include <cstddef>
#include <iostream>
#include <string_view>
#include <unordered_set>

int runStatsCustomTests() {
    static_assert(stats::CustomTypeMap.size() == static_cast<std::size_t>(stats::CustomType::count));

    int                                  failures = 0;
    std::unordered_set<std::string_view> names;
    names.reserve(stats::CustomTypeMap.size());

    for (std::size_t index = 0; index < stats::CustomTypeMap.size(); ++index) {
        auto const& [type, name] = stats::CustomTypeMap[index];
        if (static_cast<std::size_t>(type) != index) {
            std::cerr << "FAILED: CustomType index mismatch at " << index << '\n';
            ++failures;
        }
        if (name.empty()) {
            std::cerr << "FAILED: empty CustomType name at " << index << '\n';
            ++failures;
        }
        if (!names.emplace(name).second) {
            std::cerr << "FAILED: duplicate CustomType name " << name << '\n';
            ++failures;
        }
        if (stats::getCustomTypeName(type) != name) {
            std::cerr << "FAILED: CustomType lookup mismatch at " << index << '\n';
            ++failures;
        }
    }

    return failures;
}
