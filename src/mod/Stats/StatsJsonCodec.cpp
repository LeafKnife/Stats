#include "mod/Stats/StatsJsonCodec.h"

#include <nlohmann/json.hpp>

namespace stats {

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
        decoded.data.loadMap(descriptor.type, json.at(key).get<StatsDataMap>());
    }
    return decoded;
}

} // namespace stats
