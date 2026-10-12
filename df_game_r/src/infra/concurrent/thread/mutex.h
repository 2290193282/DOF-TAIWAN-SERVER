#ifndef OLDSRV_RUNTIME_MUTEX_H_
#define OLDSRV_RUNTIME_MUTEX_H_

#include <pthread.h>


// IDA: Mutex::Mutex @0x80CB412 and Mutex::~Mutex @0x80CB42E.
// The old code stores six of these at the head of MsgQueueMgr and one at the
// head of each stream pool wrapper.
class Mutex {
public:
    Mutex();
    ~Mutex();

    // IDA: Mutex::lock @0x810539E / Mutex::unlock @0x81053B2.
    int lock();
    int unlock();

    pthread_mutex_t* native();

private:
    pthread_mutex_t mutex_;
};

class MutexGuard {
public:
    explicit MutexGuard(Mutex* mutex);
    ~MutexGuard();

private:
    Mutex* mutex_;
};


#endif  // OLDSRV_RUNTIME_MUTEX_H_
