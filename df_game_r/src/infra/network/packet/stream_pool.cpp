#include "infra/network/packet/stream_pool.h"

#include "core/config/env/environment.h"
#include "infra/network/packet/stream_guard.h"

#include <utility>


template <typename T>
StreamObjectPool<T>::StreamObjectPool() : grow_count_(0), objects_(), free_() {}

template <typename T>
void StreamObjectPool<T>::init(int initial_count, int grow_count) {
    grow_count_ = grow_count;
    more_alloc(initial_count);
}

template <typename T>
void StreamObjectPool<T>::more_alloc(int count) {
    for (int index = 0; index < count; ++index) {
        std::unique_ptr<T> object(new T());
        T* raw = object.get();
        objects_.push_back(std::move(object));
        free_.push(raw);
    }
}

template <typename T>
T* StreamObjectPool<T>::acquire() {
    if (free_.empty()) {
        more_alloc(grow_count_);
    }
    if (free_.empty()) {
        return 0;
    }
    T* stream = free_.front();
    free_.pop();
    return stream;
}

template <typename T>
bool StreamObjectPool<T>::owns(Stream* stream) const {
    for (typename std::vector<std::unique_ptr<T> >::const_iterator it = objects_.begin();
         it != objects_.end(); ++it) {
        if (it->get() == stream) {
            return true;
        }
    }
    return false;
}

template <typename T>
int StreamObjectPool<T>::free(Stream* stream) {
    if (!stream || !owns(stream)) {
        return 0;
    }
    free_.push(static_cast<T*>(stream));
    return 1;
}

template <typename T>
int StreamObjectPool<T>::size() const {
    return static_cast<int>(objects_.size());
}

template <typename T>
int StreamObjectPool<T>::remain() const {
    return static_cast<int>(free_.size());
}

// IDA_EA: 0x828F974
// IDA_NAME: _ZN10StreamPoolC2Ev
// DEMANGLED: StreamPool::StreamPool(void)
StreamPool::StreamPool() : mutex_(), pool_() {
    CEnvironment* env = G_CEnvironment();
    const int count = env ? env->get_stream_pool_count() : 0;
    pool_.init(count, 1000);
}

// IDA_EA: 0x828FA20
// IDA_NAME: _ZN10StreamPoolD2Ev
// DEMANGLED: StreamPool::~StreamPool()
StreamPool::~StreamPool() {}

// IDA_EA: 0x828FA86
// IDA_NAME: _ZN10StreamPool7AcquireEPKci
// DEMANGLED: StreamPool::Acquire(char const*,int)
Stream* StreamPool::Acquire(const char* file, int line) {
    MutexGuard guard(&mutex_);
    Stream* stream = pool_.acquire();
    if (stream) {
        stream->set_acquire_site(file, line);
        stream->set_pool_kind(0);
        stream->set_big_stream(false);
        stream->set_pool_live(true);
    }
    return stream;
}

// IDA_EA: 0x828FB28
// IDA_NAME: _ZN10StreamPool4FreeEP6Stream
// DEMANGLED: StreamPool::Free(Stream *)
int StreamPool::Free(Stream* stream) {
    if (!stream) {
        return 0;
    }
    MutexGuard guard(&mutex_);
    if (!pool_.owns(stream)) {
        return 0;
    }
    stream->set_pool_live(false);
    stream->reset();
    return pool_.free(stream);
}

// IDA_EA: 0x8101080
// IDA_NAME: _ZN10StreamPool4FreeER12CStreamGuard
// DEMANGLED: StreamPool::Free(CStreamGuard &)
int StreamPool::Free(CStreamGuard& guard) {
    return Free(guard.Get());
}

// IDA_EA: 0x828FC06
// IDA_NAME: _ZN10StreamPool4SizeEv
// DEMANGLED: StreamPool::Size(void)
int StreamPool::Size() const {
    return pool_.size();
}

// IDA_EA: 0x828FC1C
// IDA_NAME: _ZN10StreamPool6RemainEv
// DEMANGLED: StreamPool::Remain(void)
int StreamPool::Remain() {
    MutexGuard guard(&mutex_);
    return pool_.remain();
}

// IDA_EA: 0x828FE62
// IDA_NAME: _ZN15SmallStreamPoolC2Ev
// DEMANGLED: SmallStreamPool::SmallStreamPool(void)
SmallStreamPool::SmallStreamPool() : mutex_(), pool_() {
    CEnvironment* env = G_CEnvironment();
    const int count = env ? env->get_small_stream_pool_count() : 0;
    pool_.init(count, 1000);
}

// IDA_EA: 0x828FF0E
// IDA_NAME: _ZN15SmallStreamPoolD2Ev
// DEMANGLED: SmallStreamPool::~SmallStreamPool()
SmallStreamPool::~SmallStreamPool() {}

// IDA_EA: 0x828FF74
// IDA_NAME: _ZN15SmallStreamPool7AcquireEPKci
// DEMANGLED: SmallStreamPool::Acquire(char const*,int)
Stream* SmallStreamPool::Acquire(const char* file, int line) {
    MutexGuard guard(&mutex_);
    Stream* stream = pool_.acquire();
    if (stream) {
        stream->set_acquire_site(file, line);
        stream->set_pool_kind(1);
        stream->set_pool_live(true);
    }
    return stream;
}

// IDA_EA: 0x8290004
// IDA_NAME: _ZN15SmallStreamPool4FreeEP6Stream
// DEMANGLED: SmallStreamPool::Free(Stream *)
int SmallStreamPool::Free(Stream* stream) {
    if (!stream) {
        return 0;
    }
    MutexGuard guard(&mutex_);
    if (!pool_.owns(stream)) {
        return 0;
    }
    stream->set_pool_live(false);
    stream->reset();
    return pool_.free(stream);
}

// IDA_EA: 0x82900E0
// IDA_NAME: _ZN15SmallStreamPool4SizeEv
// DEMANGLED: SmallStreamPool::Size(void)
int SmallStreamPool::Size() const {
    return pool_.size();
}

// IDA_EA: 0x82900F6
// IDA_NAME: _ZN15SmallStreamPool6RemainEv
// DEMANGLED: SmallStreamPool::Remain(void)
int SmallStreamPool::Remain() {
    MutexGuard guard(&mutex_);
    return pool_.remain();
}

// IDA_EA: 0x8290150
// IDA_NAME: _ZN13BigStreamPoolC2Ev
// DEMANGLED: BigStreamPool::BigStreamPool(void)
BigStreamPool::BigStreamPool() : mutex_(), pool_() {
    CEnvironment* env = G_CEnvironment();
    const int count = env ? env->get_big_stream_pool_count() : 0;
    pool_.init(count, 100);
}

// IDA_EA: 0x82901FC
// IDA_NAME: _ZN13BigStreamPoolD2Ev
// DEMANGLED: BigStreamPool::~BigStreamPool()
BigStreamPool::~BigStreamPool() {}

// IDA_EA: 0x8290262
// IDA_NAME: _ZN13BigStreamPool7AcquireEPKci
// DEMANGLED: BigStreamPool::Acquire(char const*,int)
Stream* BigStreamPool::Acquire(const char* file, int line) {
    MutexGuard guard(&mutex_);
    Stream* stream = pool_.acquire();
    if (stream) {
        stream->set_acquire_site(file, line);
        stream->set_pool_kind(2);
        stream->set_big_stream(true);
        stream->set_pool_live(true);
    }
    return stream;
}

// IDA_EA: 0x8290304
// IDA_NAME: _ZN13BigStreamPool4FreeEP6Stream
// DEMANGLED: BigStreamPool::Free(Stream *)
int BigStreamPool::Free(Stream* stream) {
    if (!stream) {
        return 0;
    }
    MutexGuard guard(&mutex_);
    if (!pool_.owns(stream)) {
        return 0;
    }
    stream->set_pool_live(false);
    stream->reset();
    return pool_.free(stream);
}

// IDA_EA: 0x82903E0
// IDA_NAME: _ZN13BigStreamPool4SizeEv
// DEMANGLED: BigStreamPool::Size(void)
int BigStreamPool::Size() const {
    return pool_.size();
}

// IDA_EA: 0x82903F6
// IDA_NAME: _ZN13BigStreamPool6RemainEv
// DEMANGLED: BigStreamPool::Remain(void)
int BigStreamPool::Remain() {
    MutexGuard guard(&mutex_);
    return pool_.remain();
}
