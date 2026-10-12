#include "dfbridge/script_thread.hpp"

#include <iostream>
#include <stdexcept>

namespace dfbridge {
namespace {

class RepositoryWorkerScope {
public:
    explicit RepositoryWorkerScope(IDChannelRepository& repository)
        : repository_(repository) {
        repository_.WorkerEnter();
    }

    ~RepositoryWorkerScope() {
        repository_.WorkerLeave();
    }

    RepositoryWorkerScope(const RepositoryWorkerScope&) = delete;
    RepositoryWorkerScope& operator=(const RepositoryWorkerScope&) = delete;

private:
    IDChannelRepository& repository_;
};

void EmitError(const ScriptThread::ErrorSink& sink, const std::string& message) {
    if (sink) {
        sink(message);
    } else {
        std::cerr << message << '\n';
    }
}

}  // namespace

ScriptThread::ScriptThread(IDChannelRepository& repository, ScriptCache& cache,
                           std::string output,
                           const std::chrono::milliseconds poll_interval,
                           ReloadSink reload_sink,
                           ErrorSink error_sink)
    : repository_(repository),
      cache_(cache),
      output_(std::move(output)),
      poll_interval_(poll_interval),
      reload_sink_(std::move(reload_sink)),
      error_sink_(std::move(error_sink)) {}

ScriptThread::~ScriptThread() {
    Stop();
}

void ScriptThread::Start() {
    if (thread_.joinable()) {
        throw std::runtime_error("ScriptThread is already running");
    }
    stop_.store(false);
    thread_ = std::thread(&ScriptThread::Loop, this);
}

void ScriptThread::Stop() {
    stop_.store(true);
    if (thread_.joinable()) {
        thread_.join();
    }
}

void ScriptThread::ReloadOnce() {
    auto snapshot = repository_.LoadSnapshot();
    ValidateSnapshot(snapshot);
    std::string script;
    bool preserve_existing_bytes = false;
    try {
        const std::string existing_bytes = ReadChannelInfoFile(output_);
        const ChannelSnapshot existing_snapshot =
            ChannelInfoParser().Parse(existing_bytes);
        if (ChannelSnapshotsSemanticallyEquivalent(existing_snapshot, snapshot)) {
            script = existing_bytes;
            preserve_existing_bytes = true;
        }
    } catch (const std::exception&) {
        // Missing or malformed cache is regenerated from the validated DB snapshot.
    }
    if (!preserve_existing_bytes) {
        script = writer_.Render(snapshot);
        writer_.WriteAtomically(snapshot, output_);
    }
    const std::string version = snapshot.script_version;
    const std::vector<GcGroup> groups = snapshot.gc_groups;
    // The registry must accept the new GC mapping before the visible cache
    // version advances; otherwise a throwing callback would suppress retries.
    if (reload_sink_) reload_sink_(version, groups);
    cache_.Replace(std::move(snapshot.script_version), script,
                   std::move(snapshot.gc_groups));
}

void ScriptThread::Loop() {
    try {
        RepositoryWorkerScope worker(repository_);
        while (!stop_.load()) {
            try {
                if (cache_.Version().empty()) {
                    ReloadOnce();
                } else {
                    const auto database_version = repository_.ReadScriptVersion();
                    if (database_version != cache_.Version()) {
                        ReloadOnce();
                    }
                }
            } catch (const std::exception& error) {
                // A transient database outage must not escape std::thread and call
                // std::terminate. Keep the last valid cache and retry next poll.
                EmitError(error_sink_, std::string("df_bridge_r ScriptThread retry: ") +
                                           error.what());
            } catch (...) {
                EmitError(error_sink_,
                          "df_bridge_r ScriptThread retry: unknown error");
            }
            std::this_thread::sleep_for(poll_interval_);
        }
    } catch (const std::exception& error) {
        EmitError(error_sink_, std::string("df_bridge_r ScriptThread worker init: ") +
                                   error.what());
    } catch (...) {
        EmitError(error_sink_, "df_bridge_r ScriptThread worker init: unknown error");
    }
}

}  // namespace dfbridge
