#ifndef OLDSRV_RUNTIME_DB_THREAD_H_
#define OLDSRV_RUNTIME_DB_THREAD_H_

#include "infra/concurrent/timer/query_counter.h"
#include "infra/concurrent/thread/thread_interface.h"

// IDA class layout (i386): ThreadInterface +0..+0x0B, log-db byte +0x0C,
// alignment +0x0D..+0x0F, then embedded CQueryCounter +0x10.
class DBThread : public ThreadInterface {
public:
    // IDA_EA: 0x83FF5F2
    // IDA_NAME: _ZN8DBThreadC2Ev
    // DEMANGLED: DBThread::DBThread(void)
    DBThread();

    // IDA_EA: 0x83FF648 (D2), 0x83FF6AA (D0)
    // IDA_NAME: _ZN8DBThreadD2Ev / _ZN8DBThreadD0Ev
    // DEMANGLED: DBThread::~DBThread()
    virtual ~DBThread();

    // IDA_EA: 0x83FF6C8
    // IDA_NAME: _ZN8DBThread8setLogDBEb
    // DEMANGLED: DBThread::setLogDB(bool)
    bool setLogDB(bool is_log_db);

    // IDA_EA: 0x83FF77A
    // IDA_NAME: _ZN8DBThread7isLogDBEv
    // DEMANGLED: DBThread::isLogDB(void)
    bool isLogDB();

    // Published only after every producer for this worker's queue has
    // stopped.  The flag lives outside this fixed-layout class so the old
    // 0x4c-byte i386 ABI remains unchanged.
    void requestStopAfterDrain();

    // IDA_EA: 0x83FF786
    // IDA_NAME: _ZN8DBThread8dispatchEPv
    // DEMANGLED: DBThread::dispatch(void *)
    // B-owned DB task loop; the canonical declaration remains in this runtime
    // ABI header, while B supplies its sole body in old_data/db.
    virtual void dispatch(void* user_data);

private:
    bool log_db_;
    char align_after_log_db_[3];
    CQueryCounter query_counter_;
};

#endif  // OLDSRV_RUNTIME_DB_THREAD_H_
