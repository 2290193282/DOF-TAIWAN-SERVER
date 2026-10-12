#ifndef OLDSRV_OLD_DATA_DB_CORE_FEATURE_DB_THREAD_H_
#define OLDSRV_OLD_DATA_DB_CORE_FEATURE_DB_THREAD_H_

#include "infra/concurrent/thread/thread_interface.h"

#include <atomic>


class FeatureDBTask;

// Dedicated SQL worker for reconstructed 0828 features.  It neither derives
// from DBThread nor owns a CQueryCounter, and consumes queue 6 only.
class FeatureDBThread : public ThreadInterface {
public:
    FeatureDBThread();
    virtual ~FeatureDBThread();

    bool ready() const;
    // Called only after every queue6 producer has stopped.  The worker exits
    // after observing an empty queue, so committed legacy fences cannot be
    // stranded during shutdown.
    void requestStopAfterDrain();
    virtual void dispatch(void* user_data);

private:
    FeatureDBTask* task_;
    std::atomic<bool> stop_after_drain_;
};


#endif  // OLDSRV_OLD_DATA_DB_CORE_NATIVE_0828_DB_THREAD_H_
