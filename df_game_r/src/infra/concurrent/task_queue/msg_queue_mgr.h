#ifndef OLDSRV_RUNTIME_MSG_QUEUE_MGR_H_
#define OLDSRV_RUNTIME_MSG_QUEUE_MGR_H_

#include "infra/network/packet/stream.h"
#include "infra/concurrent/thread/mutex.h"

#include <queue>


class CStreamGuard;

class MsgQueueMgr {
public:
    enum QUEUE_IDX {
        NETWORK_Q = 0,
        DISPATCH_Q = 1,
        DB_Q = 2,
        TIMER_Q = 3,
        LOGDB_Q = 4,
        EXCHANGE_Q = 5,
        FEATURE_DB_Q = 6,
        QUEUE_COUNT = 7,
    };

    static const char* get_queue_name(QUEUE_IDX index);

    MsgQueueMgr();
    ~MsgQueueMgr();

    int put(QUEUE_IDX index, CStreamGuard& guard);
    int put(QUEUE_IDX index, Stream* stream);
    Stream* get(QUEUE_IDX index);
    int empty(QUEUE_IDX index);
    int remain(QUEUE_IDX index);

private:
    bool valid_queue(QUEUE_IDX index) const;

    Mutex mutexes_[QUEUE_COUNT];
    std::queue<Stream*> queues_[QUEUE_COUNT];
};


#endif  // OLDSRV_RUNTIME_MSG_QUEUE_MGR_H_
