#include "infra/concurrent/task_queue/message_counter.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/statistics_log/statistics_log_payloads.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <sys/time.h>


namespace message_counter {

MessageKey::MessageKey(uint32_t signal_value,
                       uint8_t category_value,
                       uint16_t code_value)
    : signal(signal_value),
      category(category_value),
      reserved(0),
      code(code_value) {
}

bool MessageKey::operator<(const MessageKey& rhs) const {
    if (signal != rhs.signal) {
        return signal < rhs.signal;
    }
    if (category != rhs.category) {
        return category < rhs.category;
    }
    return code < rhs.code;
}

MessageValue::MessageValue()
    : count(0), total_msec(0), average_msec(0.0) {
}


// IDA_EA: 0x844DE36
// IDA_NAME: _ZN15message_counter15CMessageCounterC2Ei
CMessageCounter::CMessageCounter(int code_offset)
    : values_(), code_offset_(code_offset) {
}

// IDA_EA: 0x845000E
// IDA_NAME: _ZN15message_counter15CMessageCounterD2Ev
CMessageCounter::~CMessageCounter() {
}

// IDA_EA: 0x856F9C4
// IDA_NAME: _ZN15message_counter15CMessageCounter15IncreaseMessageEjhtx
int CMessageCounter::IncreaseMessage(uint32_t signal,
                                     uint8_t category,
                                     uint16_t code,
                                     int64_t elapsed_msec) {
    const MessageKey key(signal, category, code);
    std::map<MessageKey, MessageValue>::iterator it = values_.find(key);
    if (it == values_.end()) {
        MessageValue value;
        value.count = 1;
        value.total_msec = elapsed_msec;
        value.average_msec = static_cast<double>(elapsed_msec);
        values_.insert(std::make_pair(key, value));
        return 1;
    }

    MessageValue& value = it->second;
    ++value.count;
    value.total_msec += elapsed_msec;
    const double before = static_cast<double>(value.count - 1) /
                          static_cast<double>(value.count);
    const double current = 1.0 / static_cast<double>(value.count);
    value.average_msec = value.average_msec * before +
                         static_cast<double>(elapsed_msec) * current;
    return 1;
}

// IDA_EA: 0x856FBB0
// IDA_NAME: _ZN15message_counter15CMessageCounter8WriteLogEv
void CMessageCounter::WriteLog() {
    if (values_.empty()) {
        return;
    }

    Stream* const stream = GlobalData::stream_pool()->Acquire(
        "MessageCounter.cpp", 99);
    CStreamGuard guard(stream, false);
    guard << 108 << -1;
    SIG_INSERT_RESPONSE_TIME* payload =
        guard.GetInBuffer<SIG_INSERT_RESPONSE_TIME>();

    const int current_second = GlobalData::system_time()->getCurSec();
    const int occurrence_second = current_second - current_second % 600;
    payload->occ_time = occurrence_second;
    payload->entry_count = 0;

    for (std::map<MessageKey, MessageValue>::iterator it = values_.begin();
         it != values_.end();
         ++it) {
        const MessageKey& key = it->first;
        const MessageValue& value = it->second;
        SIG_RESPONSE_TIME_ENTRY& entry =
            payload->entries[payload->entry_count];
        entry.packet_id = key.code +
            (key.signal ? key.signal + code_offset_ : 0);
        entry.packet_count = value.count;
        entry.total_response_time_low =
            static_cast<uint32_t>(value.total_msec);
        entry.total_response_time_high = static_cast<uint32_t>(
            static_cast<uint64_t>(value.total_msec) >> 32);
        entry.avg_response_time = static_cast<uint32_t>(value.average_msec);

        if (++payload->entry_count == 100) {
            GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
            guard.Set(GlobalData::stream_pool()->Acquire(
                "MessageCounter.cpp", 123));
            guard << 108 << -1;
            payload = guard.GetInBuffer<SIG_INSERT_RESPONSE_TIME>();
            payload->occ_time = occurrence_second;
            payload->entry_count = 0;
        }
    }

    if (payload->entry_count) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    } else {
        GlobalData::stream_pool()->Free(guard);
    }
    values_.clear();
}


// IDA_EA: 0x856F974
// IDA_NAME: _ZN15message_counter13CTimeInterval6CreateEv
CTimeInterval* CTimeInterval::Create() {
    return new CUnixTimeInterval();
}

// IDA_EA: 0x856F9B0
// IDA_NAME: _ZN15message_counter13CTimeInterval7DestoryEPS0_
void CTimeInterval::Destory(CTimeInterval* interval) {
    // No virtual dtor exists in the old interface; CUnixTimeInterval only
    // owns scalar timeval fields, so the original direct operator-delete is
    // the complete lifetime operation.
    ::operator delete(interval);
}

// IDA_EA: 0x85702C6
// IDA_NAME: _ZN15message_counter13CTimeIntervalC2Ev
CTimeInterval::CTimeInterval() {
}

void CTimeInterval::SetLastTime() {
}

int64_t CTimeInterval::GetTimeInterval() {
    return 0;
}


// IDA_EA: 0x85702D4
// IDA_NAME: _ZN15message_counter17CUnixTimeIntervalC2Ev
CUnixTimeInterval::CUnixTimeInterval()
    : CTimeInterval(), last_second_(0), last_usec_(0) {
}

// IDA_EA: 0x8570236
// IDA_NAME: _ZN15message_counter17CUnixTimeInterval11SetLastTimeEv
void CUnixTimeInterval::SetLastTime() {
    timeval now;
    gettimeofday(&now, 0);
    last_second_ = static_cast<int32_t>(now.tv_sec);
    last_usec_ = static_cast<int32_t>(now.tv_usec);
}

// IDA_EA: 0x8570254
// IDA_NAME: _ZN15message_counter17CUnixTimeInterval15GetTimeIntervalEv
int64_t CUnixTimeInterval::GetTimeInterval() {
    timeval now;
    gettimeofday(&now, 0);
    return static_cast<int64_t>(now.tv_sec - last_second_) * 1000LL +
           static_cast<int64_t>(now.tv_usec - last_usec_) / 1000LL;
}


// IDA_EA: 0x847E8D2
// IDA_NAME: _ZN15message_counter18CUserPacketCounterC2EPcS1_
CUserPacketCounter::CUserPacketCounter(char* file_name,
                                       char* counter_name)
    : values_(), filename_(), total_count_(0) {
    std::sprintf(filename_, "./log/%s/%s", file_name, counter_name);
}

// IDA_EA: 0x847E916
// IDA_NAME: _ZN15message_counter18CUserPacketCounterD2Ev
CUserPacketCounter::~CUserPacketCounter() {
}

// IDA_EA: 0x856FF24
// IDA_NAME: _ZN15message_counter18CUserPacketCounter14IncreasePacketEj
CUserPacketCounter* CUserPacketCounter::IncreasePacket(uint32_t account_id) {
    std::map<uint32_t, int>::iterator it = values_.find(account_id);
    if (it == values_.end()) {
        values_.insert(std::make_pair(account_id, 1));
    } else {
        ++it->second;
    }
    ++total_count_;
    return this;
}

// IDA_EA: 0x856FFF0
// IDA_NAME: _ZN15message_counter18CUserPacketCounter8WriteLogEv
int CUserPacketCounter::WriteLog() {
    CMyFileLog total_log(
        "void message_counter::CUserPacketCounter::WriteLog()", 157);
    total_log(filename_, "user_packet_counter total_count(%d)", total_count_);
    for (std::map<uint32_t, int>::const_iterator it = values_.begin();
         it != values_.end();
         ++it) {
        CMyFileLog entry_log(
            "void message_counter::CUserPacketCounter::WriteLog()", 160);
        entry_log(filename_, "(%s,%12d)",
                  NumberToString(it->first, 0), it->second);
    }
    total_count_ = 0;
    values_.clear();
    return 1;
}

}  // namespace message_counter
