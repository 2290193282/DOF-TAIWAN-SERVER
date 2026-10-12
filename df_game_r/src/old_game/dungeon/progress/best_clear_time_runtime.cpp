#include "old_game/dungeon/progress/best_clear_time_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <utility>

// IDA_EA: 0x80C851C
// IDA_NAME: _ZN13BestClearTimeC2Ev
// DEMANGLED: BestClearTime::BestClearTime(void)
BestClearTime::BestClearTime() : best_times_(), new_record_(false) {
    best_times_.clear();
}

// IDA_EA: 0x80C8596
// IDA_NAME: _ZN13BestClearTimeD2Ev
// DEMANGLED: BestClearTime::~BestClearTime()
// IDA_EA: 0x80C85F8
// IDA_NAME: _ZN13BestClearTimeD0Ev
// DEMANGLED: BestClearTime::~BestClearTime()
BestClearTime::~BestClearTime() {}

// IDA_EA: 0x80C8616
// IDA_NAME: _ZN13BestClearTime9_saveDataEP5CUser
// DEMANGLED: BestClearTime::_saveData(CUser *)
int BestClearTime::_saveData(CUser* user) {
    Stream* stream = GlobalData::big_stream_pool()->Acquire(
        "BestClearTime.cpp", 22);
    CStreamGuard guard(stream, true);
    *guard << 0x300;
    *guard << user->GetUID();

    SIG_BEST_CLEAR_TIME* payload =
        guard->GetInBuffer<SIG_BEST_CLEAR_TIME>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    getData(reinterpret_cast<char*>(payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x80C8758
// IDA_NAME: _ZN13BestClearTime6_resetEv
// DEMANGLED: BestClearTime::_reset(void)
int BestClearTime::_reset() {
    best_times_.clear();
    new_record_ = false;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x80C8776
// IDA_NAME: _ZN13BestClearTime8loadDataEP5CUserPc
// DEMANGLED: BestClearTime::loadData(CUser *,char *)
bool BestClearTime::loadData(CUser* user, char* data) {
    (void)user;
    _reset();
    const SIG_BEST_CLEAR_TIME* payload =
        reinterpret_cast<const SIG_BEST_CLEAR_TIME*>(data);
    for (uint32_t index = 0;
         index < payload->count && index <= 999u;
         ++index) {
        const SIG_BEST_CLEAR_TIME_DATA& record = payload->records[index];
        best_times_.insert(
            std::make_pair(record.dungeon_id, record.difficulty));
    }
    return 1;
}

// IDA_EA: 0x80C883C
// IDA_NAME: _ZNK13BestClearTime7getDataEPc
// DEMANGLED: BestClearTime::getData(char *)const
void BestClearTime::getData(char* data) const {
    SIG_BEST_CLEAR_TIME* payload =
        reinterpret_cast<SIG_BEST_CLEAR_TIME*>(data);
    uint32_t count = 0;
    BestTimeMap::const_iterator it = best_times_.begin();
    for (; it != best_times_.end(); ++it) {
        if (count > 999u) {
            return;
        }
        payload->records[count].dungeon_id = it->first;
        payload->records[count].difficulty = it->second;
        ++count;
    }
    payload->count = count;
}

// IDA_EA: 0x80C8928
// IDA_NAME: _ZN13BestClearTime16setBestClearTimeEiii
// DEMANGLED: BestClearTime::setBestClearTime(int,int,int)
void BestClearTime::setBestClearTime(int dungeon_id,
                                     int difficulty,
                                     int clear_time) {
    if (difficulty <= 4) {
        if (clear_time < 0) {
            clear_time = 0;
        }

        BestTimeMap::iterator found = best_times_.find(dungeon_id);
        if (found == best_times_.end()) {
            SIG_BEST_CLEAR_TIME_DIFFICULTY value;
            value.clear_time[difficulty] = clear_time;
            best_times_.insert(std::make_pair(dungeon_id, value));
            setNewRecord(true);
            alter();
        } else if (!found->second.clear_time[difficulty] ||
                   found->second.clear_time[difficulty] > clear_time) {
            found->second.clear_time[difficulty] = clear_time;
            setNewRecord(true);
            alter();
        }
    }
}

// IDA_EA: 0x80C8AC0
// IDA_NAME: _ZN13BestClearTime16getBestClearTimeEii
// DEMANGLED: BestClearTime::getBestClearTime(int,int)
int BestClearTime::getBestClearTime(int dungeon_id, int difficulty) {
    BestTimeMap::iterator found = best_times_.find(dungeon_id);
    if (found == best_times_.end() || difficulty > 4) {
        return 0;
    }
    return found->second.clear_time[difficulty];
}

// IDA_EA: 0x80C8B44
// IDA_NAME: _ZN13BestClearTime12setNewRecordEb
// DEMANGLED: BestClearTime::setNewRecord(bool)
void BestClearTime::setNewRecord(bool new_record) {
    new_record_ = new_record;
}

// IDA_EA: 0x80C8B5C
// IDA_NAME: _ZN13BestClearTime14checkNewRecordEv
// DEMANGLED: BestClearTime::checkNewRecord(void)
bool BestClearTime::checkNewRecord() {
    return new_record_;
}
