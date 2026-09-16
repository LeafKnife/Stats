#include "mod/Stats/StatsWriteQueue.h"

#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

int runStatsWriteQueueTests() {
    std::mutex              mutex;
    std::condition_variable started;
    std::condition_variable release;
    bool                    blockerStarted  = false;
    bool                    releaseBlocker  = false;
    std::vector<stats::DecodedStats> writes;

    stats::StatsWriteQueue queue([&](stats::DecodedStats const& record) {
        if (record.info.uuid == "blocker") {
            std::unique_lock lock(mutex);
            blockerStarted = true;
            started.notify_one();
            release.wait(lock, [&] { return releaseBlocker; });
        }
        std::lock_guard lock(mutex);
        writes.push_back(record);
        return true;
    });

    queue.enqueue({{"blocker", "", ""}, {}});
    {
        std::unique_lock lock(mutex);
        started.wait(lock, [&] { return blockerStarted; });
    }

    stats::StatsData first;
    first.add(StatsType::mined, "minecraft:stone", 1);
    queue.enqueue({{"player", "xuid", "Old"}, first});

    stats::StatsData latest;
    latest.add(StatsType::mined, "minecraft:stone", 2);
    queue.enqueue({{"player", "xuid", "Latest"}, latest});

    {
        std::lock_guard lock(mutex);
        releaseBlocker = true;
    }
    release.notify_one();
    queue.flush();

    if (writes.size() != 2) {
        std::cerr << "FAILED: write queue coalesces pending player snapshots\n";
        return 1;
    }
    auto const& saved = writes.back();
    auto const* mined = saved.data.getMap(StatsType::mined);
    if (saved.info.name != "Latest" || !mined || mined->at("minecraft:stone") != 2) {
        std::cerr << "FAILED: write queue keeps the latest player snapshot\n";
        return 1;
    }

    int attempts = 0;
    stats::StatsWriteQueue failingQueue([&](stats::DecodedStats const&) -> bool {
        ++attempts;
        if (attempts == 1) throw std::runtime_error("write failed");
        return true;
    });
    failingQueue.enqueue({{"first", "", ""}, {}});
    failingQueue.flush();
    failingQueue.enqueue({{"second", "", ""}, {}});
    failingQueue.flush();
    if (attempts != 3) {
        std::cerr << "FAILED: write queue retries writer failures and remains usable\n";
        return 1;
    }
    failingQueue.stop();
    if (failingQueue.enqueue({{"late", "", ""}, {}})) {
        std::cerr << "FAILED: stopped write queue rejects new snapshots\n";
        return 1;
    }
    return 0;
}
