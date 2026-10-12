#ifndef OLDSRV_RUNTIME_GUARD_MUTEX_H_
#define OLDSRV_RUNTIME_GUARD_MUTEX_H_

#include "infra/concurrent/thread/mutex.h"


class cHistoryTrace;
class cMyTrace;

template <typename LockType>
class Guard;

// Concrete old-server specialization.  Keeping it as a full specialization
// gives the two non-weak Guard<Mutex> provider symbols from the old binary.
template <>
class Guard<Mutex> {
public:
    explicit Guard(Mutex* mutex);
    ~Guard();

private:
    friend class cHistoryTrace;
    friend class cMyTrace;

    // cHistoryTrace returns the pthread unlock result in the old binary.
    // Release keeps that return contract while the destructor remains the
    // canonical owner for every ordinary scope-exit path.
    __attribute__((always_inline)) inline int release() {
        Mutex* mutex = mutex_;
        mutex_ = 0;
        return mutex->unlock();
    }

    Mutex* mutex_;
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(Guard<Mutex>) == 0x04u,
              "Guard<Mutex> old-server ABI size must stay 0x04");
#endif


#endif  // OLDSRV_RUNTIME_GUARD_MUTEX_H_
