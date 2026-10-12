#ifndef OLDSRV_RUNTIME_TIMER_QUEUE_H_
#define OLDSRV_RUNTIME_TIMER_QUEUE_H_

#include "infra/concurrent/thread/mutex.h"

#include <stdint.h>
#include <map>
#include <queue>
#include <vector>


// IDA: TimerEntry temporary stack object built by
// TimerQueue::InsertTimerInMilisecond @0x8630E60.
struct TimerEntry {
    int obj_type;          // IDA +0: TimerEntry::OBJ_TYPE.
    int object_id;         // IDA +4: UID or object key.
    int message;           // IDA +8: TIMER_MESSAGE.
    int64_t expire_tick;   // IDA +0x0C: target millisecond tick.
    int param1;            // IDA +0x14: caller-specific parameter.
    int param2;            // IDA +0x18: caller-specific parameter.

    TimerEntry();
    ~TimerEntry();

    // IDA: TimerEntry::operator< @0x80F6364 compares expire_tick in reverse
    // order so std::priority_queue pops the earliest timer first.
    bool operator<(const TimerEntry& rhs) const;
};

// IDA: TimerQueue object layout:
//   +0x00 std::priority_queue<TimerEntry>
//   +0x10 Mutex
//   +0x28 std::map<int,int> message-count index
class TimerQueue {
public:
    TimerQueue();
    ~TimerQueue();

    // IDA: TimerQueue::InsertTimer(TimerEntry&) @0x8630CEC.
    int InsertTimer(const TimerEntry& entry);

    // IDA: TimerQueue::InsertTimer(OBJ_TYPE,int,TIMER_MESSAGE,int,uint,int)
    // @0x8630E16. Delay is expressed in seconds.
    int InsertTimer(int obj_type, int object_id, int message, int delay_sec,
                    unsigned int param1, int param2);

    // IDA: TimerQueue::InsertTimerInMilisecond @0x8630E60.
    int InsertTimerInMilisecond(int obj_type, int object_id, int message,
                                int64_t delay_msec, unsigned int param1,
                                int param2);

    // IDA: TimerQueue::GetTimerMess @0x8630ECC. Returns 1 only when the
    // earliest queued timer is due; otherwise the queue is left untouched.
    int GetTimerMess(TimerEntry* entry);

    // IDA: TimerQueue::GetQueueSize @0x847E8A4.
    int GetQueueSize() const;

    // IDA: TimerQueue::write_timer_queue_log @0x863101E.
    void write_timer_queue_log() const;

private:
    std::priority_queue<TimerEntry> timers_;
    mutable Mutex mutex_;
    std::map<int, int> message_counts_;
};

// IDA: TimerQueueSizeLog::dispatch_sig @0x8636204 /
// RegistNextTimer @0x8636348.
class TimerQueueSizeLog {
public:
    // IDA: TimerQueueSizeLog::TimerQueueSizeLog @0x863C78A.
    TimerQueueSizeLog();

    // IDA: TimerQueueSizeLog::~TimerQueueSizeLog @0x863FF18.
    ~TimerQueueSizeLog();

    // IDA: TimerQueueSizeLog::dispatch_sig(int,int,unsigned int) @0x8636204.
    int dispatch_sig(int object_id, unsigned int param1, int param2);

    // IDA: TimerQueueSizeLog::RegistNextTimer @0x8636348.
    int RegistNextTimer();
};

// IDA: G_TimerQueue @0x80F647C.
TimerQueue* G_TimerQueue();


#endif  // OLDSRV_RUNTIME_TIMER_QUEUE_H_
