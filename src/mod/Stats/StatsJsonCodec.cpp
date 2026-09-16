#include "mod/Stats/StatsJsonCodec.h"

#include <cstdint>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace stats {
namespace {

StatsDataMap decodeStatsDataMap(nlohmann::json const& document, std::string const& category) {
    auto const& values = document.at(category);
    if (!values.is_object()) throw std::invalid_argument("Statistics category must be an object: " + category);

    StatsDataMap decoded;
    decoded.reserve(values.size());
    for (auto const& [key, value] : values.items()) {
        if (value.is_number_unsigned()) {
            decoded.emplace(key, value.get<uint64_t>());
            continue;
        }
        if (!value.is_number_integer()) throw std::invalid_argument("Statistic value must be an integer: " + key);

        auto const signedValue = value.get<int64_t>();
        decoded.emplace(key, signedValue < 0 ? 0 : static_cast<uint64_t>(signedValue));
    }
    return decoded;
}

} // namespace

std::string encodeStatsJson(PlayerInfo const& info, StatsData const& data) {
    nlohmann::json json = {
        {"playerInfo", {{"uuid", info.uuid}, {"xuid", info.xuid}, {"name", info.name}}}
    };
    for (auto const& descriptor : StatsSchema) {
        auto const* values = data.getMap(descriptor.type);
        if (values) json[std::string(descriptor.key)] = *values;
    }
    return json.dump();
}

DecodedStats decodeStatsJson(std::string_view source) {
    auto const  json       = nlohmann::json::parse(source);
    auto const& playerInfo = json.at("playerInfo");

    DecodedStats decoded;
    playerInfo.at("uuid").get_to(decoded.info.uuid);
    playerInfo.at("xuid").get_to(decoded.info.xuid);
    playerInfo.at("name").get_to(decoded.info.name);
    for (auto const& descriptor : StatsSchema) {
        auto const key = std::string(descriptor.key);
        decoded.data.loadMap(descriptor.type, decodeStatsDataMap(json, key));
    }
    return decoded;
}

std::string encodeStatsPageJson(query::StatsPage const& page, std::string_view labelField) {
    nlohmann::json json = {
        {"page",       page.totalPages == 0 ? 0 : page.pageIndex + 1},
        {"pageSize",   page.pageSize                                  },
        {"total",      page.totalEntries                              },
        {"totalPages", page.totalPages                                },
        {"items",      nlohmann::json::array()                        },
    };
    for (auto const& [label, value] : page.entries) {
        json["items"].push_back({{std::string{labelField}, label}, {"value", value}});
    }
    return json.dump();
}

} // namespace stats
