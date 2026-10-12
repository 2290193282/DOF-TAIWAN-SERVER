#include "infra/concurrent/thread/guard_mutex.h"


// IDA_EA: 0x810544C
// IDA_NAME: _ZN5GuardI5MutexEC2EPS0_
// DEMANGLED: Guard<Mutex>::Guard(Mutex*)
Guard<Mutex>::Guard(Mutex* mutex) : mutex_(mutex) {
    mutex_->lock();
}

// IDA_EA: 0x8105468
// IDA_NAME: _ZN5GuardI5MutexED2Ev
// DEMANGLED: Guard<Mutex>::~Guard()
Guard<Mutex>::~Guard() {
    if (mutex_ != 0) {
        mutex_->unlock();
    }
}
