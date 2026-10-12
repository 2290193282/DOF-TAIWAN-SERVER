#pragma once

#include "dfbridge/bridge_core.hpp"
#include "dfbridge/channel_info.hpp"
#include "dfbridge/repository.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace dfbridge {

// Faithful, safer reconstruction of ChannelServiceApp::ScriptThread::loop.
class ScriptThread {
public:
    typedef std::function<void(const std::string&, const std::vector<GcGroup>&)>
        ReloadSink;
    typedef std::function<void(const std::string&)> ErrorSink;

    ScriptThread(IDChannelRepository& repository, ScriptCache& cache,
                 std::string output,
                 std::chrono::milliseconds poll_interval = std::chrono::seconds(4),
                 ReloadSink reload_sink = ReloadSink(),
                 ErrorSink error_sink = ErrorSink());
    ~ScriptThread();

    ScriptThread(const ScriptThread&) = delete;
    ScriptThread& operator=(const ScriptThread&) = delete;

    void Start();
    void Stop();
    void ReloadOnce();

private:
    void Loop();

    IDChannelRepository& repository_;
    ScriptCache& cache_;
    std::string output_;
    std::chrono::milliseconds poll_interval_;
    ChannelInfoWriter writer_;
    ReloadSink reload_sink_;
    ErrorSink error_sink_;
    std::atomic<bool> stop_{false};
    std::thread thread_;
};

}  // namespace dfbridge
