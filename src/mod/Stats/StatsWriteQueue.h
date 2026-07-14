#pragma once

#include "mod/Stats/StatsJsonCodec.h"

#include <functional>
#include <memory>

namespace stats {

class StatsWriteQueue {
public:
    using Writer = std::function<bool(DecodedStats const&)>;

    explicit StatsWriteQueue(Writer writer);
    ~StatsWriteQueue();

    StatsWriteQueue(StatsWriteQueue const&)            = delete;
    StatsWriteQueue& operator=(StatsWriteQueue const&) = delete;

    bool enqueue(DecodedStats record);
    void flush();
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace stats
