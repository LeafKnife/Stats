#include "mod/Stats/StatsWriteQueue.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

namespace stats {

struct StatsWriteQueue::Impl {
    explicit Impl(Writer callback) : writer(std::move(callback)), worker([this] { run(); }) {}

    void run() {
        while (true) {
            DecodedStats record;
            {
                std::unique_lock lock(mutex);
                available.wait(lock, [this] { return stopping || !pending.empty(); });
                if (pending.empty()) break;

                auto entry = pending.extract(pending.begin());
                record     = std::move(entry.mapped());
                writing    = true;
            }

            for (int attempt = 0; attempt < 3; ++attempt) {
                try {
                    if (writer(record)) break;
                } catch (...) {
                    // Keep the worker alive so shutdown and later snapshots can still complete.
                }
                if (attempt < 2) std::this_thread::sleep_for(std::chrono::milliseconds{25});
            }

            {
                std::lock_guard lock(mutex);
                writing = false;
                if (pending.empty()) drained.notify_all();
            }
        }

        std::lock_guard lock(mutex);
        drained.notify_all();
    }

    Writer                                        writer;
    std::mutex                                    mutex;
    std::condition_variable                       available;
    std::condition_variable                       drained;
    std::unordered_map<std::string, DecodedStats> pending;
    bool                                          writing  = false;
    bool                                          stopping = false;
    std::thread                                   worker;
};

StatsWriteQueue::StatsWriteQueue(Writer writer) : mImpl(std::make_unique<Impl>(std::move(writer))) {}

StatsWriteQueue::~StatsWriteQueue() { stop(); }

bool StatsWriteQueue::enqueue(DecodedStats record) {
    std::lock_guard lock(mImpl->mutex);
    if (mImpl->stopping) return false;

    auto uuid = record.info.uuid;
    mImpl->pending.insert_or_assign(std::move(uuid), std::move(record));
    mImpl->available.notify_one();
    return true;
}

void StatsWriteQueue::flush() {
    std::unique_lock lock(mImpl->mutex);
    mImpl->drained.wait(lock, [this] { return mImpl->pending.empty() && !mImpl->writing; });
}

void StatsWriteQueue::stop() {
    if (!mImpl || !mImpl->worker.joinable()) return;
    {
        std::lock_guard lock(mImpl->mutex);
        mImpl->stopping = true;
    }
    mImpl->available.notify_one();
    mImpl->worker.join();
}

} // namespace stats
