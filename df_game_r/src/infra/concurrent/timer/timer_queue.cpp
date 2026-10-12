#include "infra/concurrent/timer/timer_queue.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_instance.h"
#include "infra/log/format/clog.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <sstream>


// IDA_EA: 0x847E85C
// IDA_NAME: _ZN10TimerEntryC2Ev
// DEMANGLED: TimerEntry::TimerEntry(void)
TimerEntry::TimerEntry()
    : obj_type(0), object_id(0), message(117), expire_tick(0), param1(0), param2(0) {}

// IDA_EA: 0x863DD96
// IDA_NAME: _ZN10TimerEntryD2Ev
// DEMANGLED: TimerEntry::~TimerEntry()
TimerEntry::~TimerEntry() {}

// IDA_EA: 0x80F6364
// IDA_NAME: _ZNK10TimerEntryltERKS_
// DEMANGLED: TimerEntry::operator<(TimerEntry const&) const
bool TimerEntry::operator<(const TimerEntry& rhs) const {
    return expire_tick >= rhs.expire_tick;
}

// IDA_EA: 0x80F63AC
// IDA_NAME: _ZN10TimerQueueC2Ev
// DEMANGLED: TimerQueue::TimerQueue(void)
TimerQueue::TimerQueue() : timers_(), mutex_(), message_counts_() {}

// IDA_EA: 0x82A42B8
// IDA_NAME: _ZN10TimerQueueD2Ev
// DEMANGLED: TimerQueue::~TimerQueue()
TimerQueue::~TimerQueue() {}

// IDA_EA: 0x8630CEC
// IDA_NAME: _ZN10TimerQueue11InsertTimerER10TimerEntry
// DEMANGLED: TimerQueue::InsertTimer(TimerEntry &)
int TimerQueue::InsertTimer(const TimerEntry& entry) {
    MutexGuard guard(&mutex_);
    timers_.push(entry);
    std::map<int, int>::iterator it = message_counts_.find(entry.message);
    if (it != message_counts_.end()) {
        ++it->second;
    } else {
        message_counts_.insert(std::make_pair(entry.message, 1));
    }
    return 0;
}

// IDA_EA: 0x8630E16
// IDA_NAME: _ZN10TimerQueue11InsertTimerEN10TimerEntry8OBJ_TYPEEi13TIMER_MESSAGEiij
// DEMANGLED: TimerQueue::InsertTimer(TimerEntry::OBJ_TYPE,int,TIMER_MESSAGE,int,int,unsigned int)
int TimerQueue::InsertTimer(int obj_type, int object_id, int message, int delay_sec,
                            unsigned int param1, int param2) {
    return InsertTimerInMilisecond(obj_type, object_id, message,
                                   static_cast<int64_t>(delay_sec) * 1000,
                                   param1, param2);
}

// IDA_EA: 0x8630E60
// IDA_NAME: _ZN10TimerQueue23InsertTimerInMilisecondEN10TimerEntry8OBJ_TYPEEi13TIMER_MESSAGExij
// DEMANGLED: TimerQueue::InsertTimerInMilisecond(TimerEntry::OBJ_TYPE,int,TIMER_MESSAGE,long long,int,unsigned int)
int TimerQueue::InsertTimerInMilisecond(int obj_type, int object_id, int message,
                                        int64_t delay_msec, unsigned int param1,
                                        int param2) {
    TimerEntry entry;
    entry.obj_type = obj_type;
    entry.object_id = object_id;
    entry.message = message;
    CSystemTime* system_time = GlobalData::system_time();
    entry.expire_tick = delay_msec +
        static_cast<unsigned int>(system_time ? system_time->getCurTickCount() : 0);
    entry.param1 = static_cast<int>(param1);
    entry.param2 = param2;
    return InsertTimer(entry);
}

// IDA_EA: 0x8630ECC
// IDA_NAME: _ZN10TimerQueue12GetTimerMessER10TimerEntry
// DEMANGLED: TimerQueue::GetTimerMess(TimerEntry &)
int TimerQueue::GetTimerMess(TimerEntry* entry) {
    if (entry == 0) {
        return 0;
    }

    CSystemTime* system_time = GlobalData::system_time();
    const unsigned int cur_tick =
        static_cast<unsigned int>(system_time ? system_time->getCurTickCount() : 0);

    MutexGuard guard(&mutex_);
    if (timers_.empty()) {
        return 0;
    }

    *entry = timers_.top();
    if (entry->expire_tick > static_cast<int64_t>(cur_tick)) {
        return 0;
    }

    timers_.pop();
    std::map<int, int>::iterator it = message_counts_.find(entry->message);
    if (it != message_counts_.end()) {
        --it->second;
    }
    return 1;
}

// IDA_EA: 0x847E8A4
// IDA_NAME: _ZN10TimerQueue12GetQueueSizeEv
// DEMANGLED: TimerQueue::GetQueueSize(void)
int TimerQueue::GetQueueSize() const {
    MutexGuard guard(&mutex_);
    return static_cast<int>(timers_.size());
}

// IDA_EA: 0x863101E
// IDA_NAME: _ZN10TimerQueue21write_timer_queue_logEv
// DEMANGLED: TimerQueue::write_timer_queue_log(void)
void TimerQueue::write_timer_queue_log() const {
    MutexGuard guard(&mutex_);
    std::ostringstream chunk;
    for (std::map<int, int>::const_iterator it = message_counts_.begin();
         it != message_counts_.end(); ++it) {
        chunk << "(" << it->first << " " << it->second << ") ";
        if (chunk.str().size() > 0x800u) {
            CLog* log = CLog::instance();
            if (log != 0) {
                log->logNotice("TIMER_QUEUE_LOG : %s", chunk.str().c_str());
            }
            chunk.str("");
            chunk.clear();
        }
    }

    const std::string text = chunk.str();
    if (!text.empty()) {
        CLog* log = CLog::instance();
        if (log != 0) {
            log->logNotice("TIMER_QUEUE_LOG : %s", text.c_str());
        }
    }
}

// IDA_EA: 0x863C78A
// IDA_NAME: _ZN17TimerQueueSizeLogC2Ev
// DEMANGLED: TimerQueueSizeLog::TimerQueueSizeLog(void)
TimerQueueSizeLog::TimerQueueSizeLog() {}

// IDA_EA: 0x863FF18
// IDA_NAME: _ZN17TimerQueueSizeLogD2Ev
// DEMANGLED: TimerQueueSizeLog::~TimerQueueSizeLog()
TimerQueueSizeLog::~TimerQueueSizeLog() {}

// IDA_EA: 0x863FF48
// IDA_NAME: _ZN17TimerQueueSizeLogD0Ev
// DEMANGLED: TimerQueueSizeLog::~TimerQueueSizeLog()
// The original deleting destructor is compiler-generated from the C++ destructor.

// IDA_EA: 0x8636204
// IDA_NAME: _ZN17TimerQueueSizeLog12dispatch_sigEiij
// DEMANGLED: TimerQueueSizeLog::dispatch_sig(int,int,unsigned int)
int TimerQueueSizeLog::dispatch_sig(int object_id,
                                    unsigned int param1,
                                    int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;
    MsgQueueMgr* msgq = GlobalData::msgq_mgr();
    if (msgq != 0 && msgq->remain(MsgQueueMgr::DB_Q) > 10) {
        StreamPool* pool = GlobalData::stream_pool();
        if (pool != 0) {
            Stream* stream = pool->Acquire("TimerDispatcher.cpp", 2687);
            CStreamGuard guard(stream, true);
            guard << 155;
            guard << -1;
            guard << static_cast<char>(msgq->remain(MsgQueueMgr::DB_Q));
            CEnvironment* env = G_CEnvironment();
            guard << static_cast<char>(env ? env->get_channel_no() : 0);
            msgq->put(MsgQueueMgr::LOGDB_Q, guard);
        }
    }
    return RegistNextTimer() == 1 ? 1 : 0;
}

// IDA_EA: 0x8636348
// IDA_NAME: _ZN17TimerQueueSizeLog15RegistNextTimerEv
// DEMANGLED: TimerQueueSizeLog::RegistNextTimer(void)
int TimerQueueSizeLog::RegistNextTimer() {
    TimerQueue* timer_queue = G_TimerQueue();
    if (timer_queue != 0) {
        timer_queue->InsertTimer(2, 0, 97, 60, 0, 0);
    }
    return 1;
}

// IDA_EA: 0x80F647C
// IDA_NAME: _Z12G_TimerQueuev
// DEMANGLED: G_TimerQueue(void)
TimerQueue* G_TimerQueue() {
    return GlobalInstance<TimerQueue>::inst_ptr();
}
