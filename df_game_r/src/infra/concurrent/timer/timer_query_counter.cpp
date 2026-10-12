#include "infra/concurrent/timer/timer_query_counter.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/timer/timer_queue.h"


#if defined(__i386__)
typedef char TimerQueryCounter_Size_Must_Be_4[
    (sizeof(TimerQueryCounter) == 4) ? 1 : -1];
#endif


// IDA_EA: 0x863C68E
// IDA_NAME: _ZN17TimerQueryCounterC2Ev
// DEMANGLED: TimerQueryCounter::TimerQueryCounter(void)
TimerQueryCounter::TimerQueryCounter() : TimerSig() {}

// IDA_EA: 0x864013A
// IDA_NAME: _ZN17TimerQueryCounterD2Ev
// DEMANGLED: TimerQueryCounter::~TimerQueryCounter()
// IDA_EA: 0x864016A
// IDA_NAME: _ZN17TimerQueryCounterD0Ev
// DEMANGLED: TimerQueryCounter::~TimerQueryCounter()
TimerQueryCounter::~TimerQueryCounter() {}

// IDA_EA: 0x8635A60
// IDA_NAME: _ZN17TimerQueryCounter12dispatch_sigEiij
// DEMANGLED: TimerQueryCounter::dispatch_sig(int,int,unsigned int)
int TimerQueryCounter::dispatch_sig(int object_id, int param1,
                                    unsigned int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;

    CStreamGuard logdb_guard(
        GlobalData::s_stream_pool->Acquire("TimerDispatcher.cpp", 2427), true);
    logdb_guard << 123;
    logdb_guard << -1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, logdb_guard);

    CStreamGuard db_guard(
        GlobalData::s_stream_pool->Acquire("TimerDispatcher.cpp", 2433), true);
    db_guard << 124;
    db_guard << -1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, db_guard);

    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x8635BDC
// IDA_NAME: _ZN17TimerQueryCounter15RegistNextTimerEv
// DEMANGLED: TimerQueryCounter::RegistNextTimer(void)
int TimerQueryCounter::RegistNextTimer() {
    G_TimerQueue()->InsertTimer(2, 0, 89, 1800, 0, 0);
    return 1;
}
