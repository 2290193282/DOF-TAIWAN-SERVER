#include "old_data/db/core/feature_db_thread.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/feature_db_task.h"

#include <iostream>
#include <new>


FeatureDBThread::FeatureDBThread()
    : ThreadInterface(),
      task_(new (std::nothrow) FeatureDBTask()),
      stop_after_drain_(false) {}

FeatureDBThread::~FeatureDBThread() {
    delete task_;
    task_ = 0;
}

bool FeatureDBThread::ready() const {
    return task_ && task_->ready();
}

void FeatureDBThread::requestStopAfterDrain() {
    stop_after_drain_.store(true, std::memory_order_release);
}

void FeatureDBThread::dispatch(void* user_data) {
    (void)user_data;

    DBMgr::SetFeatureDBThreadContext(true);
    if (!ready()) {
        std::cerr << "[Native0828DBThread::dispatch] "
                     "dispatcher registration failed"
                  << std::endl;
        DBMgr::SetFeatureDBThreadContext(false);
        return;
    }

    for (;;) {
        bool had_event = false;
        if (task_->HandleEvents(had_event) != 1) {
            break;
        }
        // Only an actual empty-queue observation may complete the drain.
        // Malformed/truncated messages still set had_event=true, so they can
        // never strand valid work that was queued behind them.
        if (!had_event &&
            stop_after_drain_.load(std::memory_order_acquire)) {
            break;
        }
    }
    DBMgr::SetFeatureDBThreadContext(false);
}
