#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_TASK_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_TASK_H_

#include "infra/concurrent/task_queue/msg_queue_mgr.h"


class DBDispatcher;
class Stream;

// IDA unit: 99_misc_global/DBTask.
class DBTask {
public:
    DBTask();
    ~DBTask();

    int RegistDispatcher();
    int UnregistDispatcher();
    int HandleEvents(MsgQueueMgr::QUEUE_IDX queue_idx, int& signal);
    int DBProcess(Stream* stream, int& signal);

private:
    enum {
        kDispatcherCount = 852,
    };

    MsgQueueMgr::QUEUE_IDX queue_idx_;
    DBDispatcher* dispatchers_[kDispatcherCount];
};

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_TASK_H_
