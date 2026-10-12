#ifndef OLDSRV_PROTOCOL_STREAM_POOL_H_
#define OLDSRV_PROTOCOL_STREAM_POOL_H_

#include "infra/network/packet/stream.h"
#include "infra/concurrent/thread/mutex.h"

#include <memory>
#include <queue>
#include <vector>


class CStreamGuard;

template <typename T>
class StreamObjectPool {
public:
    StreamObjectPool();
    void init(int initial_count, int grow_count);
    T* acquire();
    bool owns(Stream* stream) const;
    int free(Stream* stream);
    int size() const;
    int remain() const;

private:
    void more_alloc(int count);

    int grow_count_;
    std::vector<std::unique_ptr<T> > objects_;
    std::queue<T*> free_;
};

class StreamPool {
public:
    StreamPool();
    ~StreamPool();

    Stream* Acquire(const char* file, int line);
    int Free(Stream* stream);
    int Free(CStreamGuard& guard);
    int Size() const;
    int Remain();

private:
    Mutex mutex_;
    StreamObjectPool<Stream> pool_;
};

class SmallStreamPool {
public:
    SmallStreamPool();
    ~SmallStreamPool();

    Stream* Acquire(const char* file, int line);
    int Free(Stream* stream);
    int Size() const;
    int Remain();

private:
    Mutex mutex_;
    StreamObjectPool<SmallStream> pool_;
};

class BigStreamPool {
public:
    BigStreamPool();
    ~BigStreamPool();

    Stream* Acquire(const char* file, int line);
    int Free(Stream* stream);
    int Size() const;
    int Remain();

private:
    Mutex mutex_;
    StreamObjectPool<BigStream> pool_;
};


#endif  // OLDSRV_PROTOCOL_STREAM_POOL_H_
