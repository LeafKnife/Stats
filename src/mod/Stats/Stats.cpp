#include "mod/Stats/Stats.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/PropertiesSettings.h>


#include "mod/Events/Events.h"
#include "mod/Hook/Hook.h"
#include "mod/MyMod.h"
#include "mod/Stats/Command/RegisterCommand.h"
#include "mod/Stats/StatsType.h"

using namespace ll::i18n_literals;

namespace stats {
namespace {
PlayerStatsMap playerStatsMap;
StatsCache     statsCache;
std::string    levelName;
} // namespace

ll::io::Logger& getLogger() { return lk::MyMod::getInstance().getSelf().getLogger(); }
PlayerStatsMap& getPlayerStatsMap() { return playerStatsMap; }
StatsCache&     getStatsCache() { return statsCache; }

StatsCacheData parseStatsData(const std::string& data) {
    auto j             = nlohmann::json::parse(data);
    auto tmpData       = std::make_shared<StatsData>();
    auto tmpInfo       = PlayerInfo();
    tmpInfo.name       = j["playerInfo"]["name"];
    tmpInfo.uuid       = j["playerInfo"]["uuid"];
    tmpInfo.xuid       = j["playerInfo"]["xuid"];
    tmpData->custom    = j["minecraft:custom"];
    tmpData->mined     = j["minecraft:mined"];
    tmpData->broken    = j["minecraft:broken"];
    tmpData->crafted   = j["minecraft:crafted"];
    tmpData->used      = j["minecraft:used"];
    tmpData->picked_up = j["minecraft:picked_up"];
    tmpData->dropped   = j["minecraft:dropped"];
    tmpData->killed    = j["minecraft:killed"];
    tmpData->killed_by = j["minecraft:killed_by"];
    StatsCacheData r   = std::make_pair(tmpInfo, tmpData);
    return r;
}

inline std::optional<std::string> getLevelName() {
    if (ll::service::getPropertiesSettings().has_value()) {
        return ll::service::getPropertiesSettings()->mLevelName;
    } else {
        std::ifstream fin("server.properties");
        std::string   buf;
        while (getline(fin, buf)) {
            if (buf.find("level-name=") != std::string::npos) {
                if (buf.back() == '\n') buf.pop_back();
                if (buf.back() == '\r') buf.pop_back();
                return buf.substr(11);
            }
        }
    }
    return std::nullopt;
}
std::filesystem::path getStatsPath() {
    if (!levelName.empty()) {
        return ll::file_utils::u8path("./worlds/" + levelName + "/stats");
    } else {
        auto name = getLevelName();
        if (name.has_value()) {
            levelName = name.value();
        }
        return ll::file_utils::u8path("./worlds/" + levelName + "/stats");
    }
}

bool loadStatsCache() {
    auto        oldPath      = ll::file_utils::u8path("./stats");
    auto        newPath      = getStatsPath();
    std::string extension    = ".json";
    auto        existOldPath = std::filesystem::exists(oldPath);
    auto        existPath    = std::filesystem::exists(newPath);
    if (!existPath) {
        if (existOldPath) {
            getLogger().warn("log.info.ExistOldPath"_tr());
            try {
                std::filesystem::rename(oldPath, newPath);
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("log.warn.moveStats.fail"_tr());
                return false;
            }
        } else {
            getLogger().warn("log.info.CreateStatsPath"_tr());
            try {
                std::filesystem::create_directory(newPath);
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("log.warn.CreatePath.fail"_tr());
                return false;
            }
        }
    }
    for (const auto& entry : std::filesystem::directory_iterator(newPath)) {
        // 检查文件是否为所需后缀
        if (entry.path().extension() == extension) {
            auto rawData = ll::file_utils::readFile(entry.path());
            try {
                auto data = parseStatsData(*rawData);
                statsCache.push_back(data);
            } catch (std::exception& excep) {
                getLogger().error(excep.what());
                getLogger().warn("data.parse.fail"_tr(entry.path().filename()));
            }
        }
    }
    return true;
}

void load() {
    if (loadStatsCache()) {
        event::listenEvents();
        hook::hook();
        command::registerCommand();
        getLogger().info("plugins.load.success"_tr());
    } else {
        getLogger().warn("plugins.load.fail"_tr());
    }
}

void unload() {
    event::removeEvents();
    hook::unhook();
}

void printLogo() {
    auto& logger = getLogger();
    logger.info(R"(                                                                         )");
    logger.info(R"(     ██╗     ██╗  ██╗     ███████╗████████╗ █████╗ ████████╗███████╗     )");
    logger.info(R"(     ██║     ██║ ██╔╝     ██╔════╝╚══██╔══╝██╔══██╗╚══██╔══╝██╔════╝     )");
    logger.info(R"(     ██║     █████╔╝█████╗███████╗   ██║   ███████║   ██║   ███████╗     )");
    logger.info(R"(     ██║     ██╔═██╗╚════╝╚════██║   ██║   ██╔══██║   ██║   ╚════██║     )");
    logger.info(R"(     ███████╗██║  ██╗     ███████║   ██║   ██║  ██║   ██║   ███████║     )");
    logger.info(R"(     ╚══════╝╚═╝  ╚═╝     ╚══════╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚══════╝     )");
    logger.info(R"(                                                                         )");
    /// logger.info(R"(     ----------------   LeafKnife Statistics Plugin  ---------------     )");
    logger.info("Repository: https://github.com/LeafKnife/Stats");
}
} // namespace stats