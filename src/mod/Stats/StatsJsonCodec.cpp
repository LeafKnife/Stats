#include "mod/Stats/StatsJsonCodec.h"

#include <nlohmann/json.hpp>

namespace stats {

std::string encodeStatsJson(PlayerInfo const& info, StatsData const& data) {
    nlohmann::json json = {
        {"playerInfo",          {{"uuid", info.uuid}, {"xuid", info.xuid}, {"name", info.name}}},
        {"minecraft:custom",    data.custom.asMap()                                            },
        {"minecraft:mined",     data.mined                                                     },
        {"minecraft:broken",    data.broken                                                    },
        {"minecraft:crafted",   data.crafted                                                   },
        {"minecraft:used",      data.used                                                      },
        {"minecraft:picked_up", data.picked_up                                                 },
        {"minecraft:dropped",   data.dropped                                                   },
        {"minecraft:killed",    data.killed                                                    },
        {"minecraft:killed_by", data.killed_by                                                 },
    };
    return json.dump();
}

DecodedStats decodeStatsJson(std::string_view source) {
    auto const  json       = nlohmann::json::parse(source);
    auto const& playerInfo = json.at("playerInfo");

    DecodedStats decoded;
    playerInfo.at("uuid").get_to(decoded.info.uuid);
    playerInfo.at("xuid").get_to(decoded.info.xuid);
    playerInfo.at("name").get_to(decoded.info.name);
    decoded.data.custom.load(json.at("minecraft:custom").get<StatsDataMap>());
    json.at("minecraft:mined").get_to(decoded.data.mined);
    json.at("minecraft:broken").get_to(decoded.data.broken);
    json.at("minecraft:crafted").get_to(decoded.data.crafted);
    json.at("minecraft:used").get_to(decoded.data.used);
    json.at("minecraft:picked_up").get_to(decoded.data.picked_up);
    json.at("minecraft:dropped").get_to(decoded.data.dropped);
    json.at("minecraft:killed").get_to(decoded.data.killed);
    json.at("minecraft:killed_by").get_to(decoded.data.killed_by);
    return decoded;
}

} // namespace stats
