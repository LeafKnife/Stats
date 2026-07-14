#include "mod/Stats/StatsFileRepository.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/PropertiesSettings.h>

#include <Windows.h>

#include "mod/Stats/Stats.h"
#include "mod/Stats/StatsWriteQueue.h"

using namespace ll::i18n_literals;

namespace stats::repository {
namespace {

std::filesystem::path statsPath;
std::unique_ptr<StatsWriteQueue> writeQueue;

std::optional<std::string> getLevelName() {
    if (auto const settings = ll::service::getPropertiesSettings()) {
        return settings->mLevelName;
    }

    std::ifstream file("server.properties");
    std::string   line;
    while (std::getline(file, line)) {
        constexpr std::string_view prefix = "level-name=";
        if (!line.starts_with(prefix)) continue;
        auto name = line.substr(prefix.size());
        if (!name.empty() && name.back() == '\r') name.pop_back();
        return name;
    }
    return std::nullopt;
}

std::filesystem::path resolveStatsPath() {
    auto const levelName = getLevelName().value_or("");
    return ll::file_utils::u8path("./worlds/" + levelName + "/stats");
}

bool isValidSnapshot(std::filesystem::path const& path) {
    auto const source = ll::file_utils::readFile(path);
    if (!source) return false;

    try {
        (void)decodeStatsJson(*source);
        return true;
    } catch (...) {
        return false;
    }
}

void recoverPendingWrites() {
    for (auto const& entry : std::filesystem::directory_iterator(statsPath)) {
        auto const& tempPath = entry.path();
        if (tempPath.extension() != ".tmp" || tempPath.stem().extension() != ".json") continue;

        if (!isValidSnapshot(tempPath)) {
            getLogger().warn("Ignoring invalid temporary stats snapshot: {}", tempPath.string());
            continue;
        }

        auto targetPath = tempPath;
        targetPath.replace_extension();

        bool recover = !std::filesystem::exists(targetPath) || !isValidSnapshot(targetPath);
        if (!recover) {
            std::error_code timeError;
            auto const      tempTime = std::filesystem::last_write_time(tempPath, timeError);
            if (!timeError) {
                auto const targetTime = std::filesystem::last_write_time(targetPath, timeError);
                if (!timeError) recover = tempTime > targetTime;
            }
            if (timeError) {
                getLogger().warn("Could not compare pending stats snapshot times: {}", tempPath.string());
                continue;
            }
        }

        if (!recover) {
            std::error_code removeError;
            std::filesystem::remove(tempPath, removeError);
            if (removeError) {
                getLogger().warn("Failed to remove stale stats snapshot: {}", tempPath.string());
            }
            continue;
        }

        if (MoveFileExW(
                tempPath.c_str(),
                targetPath.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
            )) {
            getLogger().info("Recovered pending stats snapshot: {}", targetPath.string());
        } else {
            getLogger().error(
                "Failed to recover pending stats snapshot: {} ({})",
                targetPath.string(),
                GetLastError()
            );
        }
    }
}

bool writeNow(DecodedStats const& record) {
    try {
        auto const path = statsPath / ll::file_utils::u8path(record.info.uuid + ".json");
        auto       tempPath = path;
        tempPath += ".tmp";
        if (!ll::file_utils::writeFile(tempPath, encodeStatsJson(record.info, record.data))) {
            getLogger().error("Failed to write stats snapshot: {}", path.string());
            return false;
        }
        if (MoveFileExW(
                tempPath.c_str(),
                path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
            )) {
            return true;
        }
        auto const replaceError = GetLastError();
        getLogger().error("Failed to replace stats snapshot: {} ({})", path.string(), replaceError);
        return false;
    } catch (std::exception const& exception) {
        getLogger().error(exception.what());
        return false;
    }
}

void ensureWriteQueue() {
    if (!writeQueue) writeQueue = std::make_unique<StatsWriteQueue>(writeNow);
}

} // namespace

bool initialize() {
    if (!statsPath.empty()) {
        ensureWriteQueue();
        return true;
    }

    auto const oldPath = ll::file_utils::u8path("./stats");
    auto const newPath = resolveStatsPath();
    if (!std::filesystem::exists(newPath)) {
        if (std::filesystem::exists(oldPath)) {
            getLogger().warn("log.info.ExistOldPath"_tr());
            try {
                std::filesystem::rename(oldPath, newPath);
            } catch (std::exception const& exception) {
                getLogger().error(exception.what());
                getLogger().warn("log.warn.moveStats.fail"_tr());
                return false;
            }
        } else {
            getLogger().warn("log.info.CreateStatsPath"_tr());
            try {
                std::filesystem::create_directory(newPath);
            } catch (std::exception const& exception) {
                getLogger().error(exception.what());
                getLogger().warn("log.warn.CreatePath.fail"_tr());
                return false;
            }
        }
    }

    statsPath = newPath;
    ensureWriteQueue();
    return true;
}

bool loadAll(std::vector<DecodedStats>& records) {
    records.clear();
    std::vector<std::filesystem::path> files;
    try {
        recoverPendingWrites();
        for (auto const& entry : std::filesystem::directory_iterator(statsPath)) {
            if (entry.path().extension() == ".json") files.push_back(entry.path());
        }
    } catch (std::exception const& exception) {
        getLogger().error(exception.what());
        return false;
    }

    records.reserve(files.size());
    for (auto const& path : files) {
        auto const source = ll::file_utils::readFile(path);
        if (!source) {
            getLogger().warn("data.parse.fail"_tr(path.filename()));
            continue;
        }
        try {
            records.push_back(decodeStatsJson(*source));
        } catch (std::exception const& exception) {
            getLogger().error(exception.what());
            getLogger().warn("data.parse.fail"_tr(path.filename()));
        }
    }
    return true;
}

bool save(PlayerInfo const& info, StatsData const& data) {
    if (!writeQueue) return false;
    return writeQueue->enqueue(DecodedStats{info, data});
}

void flush() {
    if (writeQueue) writeQueue->flush();
}

void shutdown() { writeQueue.reset(); }

} // namespace stats::repository
