#include "infra/concurrent/thread/mutex.h"


// IDA_EA: 0x80CB412
// IDA_NAME: _ZN5MutexC2Ev
// DEMANGLED: Mutex::Mutex(void)
Mutex::Mutex() {
    pthread_mutex_init(&mutex_, 0);
}

// IDA_EA: 0x80CB42E
// IDA_NAME: _ZN5MutexD2Ev
// DEMANGLED: Mutex::~Mutex()
Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex_);
}

// IDA_EA: 0x810539E
// IDA_NAME: _ZN5Mutex4lockEv
// DEMANGLED: Mutex::lock(void)
int Mutex::lock() {
    return pthread_mutex_lock(&mutex_);
}

// IDA_EA: 0x81053B2
// IDA_NAME: _ZN5Mutex6unlockEv
// DEMANGLED: Mutex::unlock(void)
int Mutex::unlock() {
    return pthread_mutex_unlock(&mutex_);
}

pthread_mutex_t* Mutex::native() {
    return &mutex_;
}

MutexGuard::MutexGuard(Mutex* mutex) : mutex_(mutex) {
    if (mutex_) {
        mutex_->lock();
    }
}

MutexGuard::~MutexGuard() {
    if (mutex_) {
        mutex_->unlock();
    }
}
