#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"

#include <cstring>
#include <ctime>

#include "common/manager/global_data.h"
#include "old_data/db/dungeon/blood_dungeon_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/blood_dungeon_stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

long getDailyInitializeTime(int hour, int minute, int second);

// IDA_EA: 0x832A2D4
// IDA_NAME: _ZN19CCharacBloodDungeon6_resetEv
// DEMANGLED: CCharacBloodDungeon::_reset(void)
int CCharacBloodDungeon::_reset() {
    week_point_ = 0;
    week_enter_count_ = 0;
    week_use_gold_ = 0;
    last_play_date_ = 0;
    enter_count_ = 0;
    previous_rank_ = 0;
    previous_reward_claimed_ = 0;
    previous_reward_item_id_ = 0;
    previous_reward_gold_ = 0;
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x832A330
// IDA_NAME: _ZN19CCharacBloodDungeon9_saveDataEP5CUser
// DEMANGLED: CCharacBloodDungeon::_saveData(CUser *)
int CCharacBloodDungeon::_saveData(CUser* user) {
    Stream* stream =
        GlobalData::s_stream_pool->Acquire("CharacBloodDungeon.cpp", 30);
    CStreamGuard guard(stream, true);
    (*guard) << 0x197;
    (*guard) << user->GetUID();

    SIG_BLOOD_DUNGEON* const payload =
        guard->GetInBuffer<SIG_BLOOD_DUNGEON>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    payload->week_point = static_cast<uint32_t>(week_point_);
    payload->week_enter_count = static_cast<uint32_t>(week_enter_count_);
    payload->week_use_gold = static_cast<uint32_t>(week_use_gold_);
    payload->last_play_date =
        static_cast<int32_t>(getDailyInitializeTime(6, 0, 0));
    payload->enter_count = static_cast<uint32_t>(enter_count_);
    payload->previous_rank = static_cast<uint32_t>(previous_rank_);
    payload->previous_reward_claimed = previous_reward_claimed_;
    payload->previous_reward_item_id =
        static_cast<uint32_t>(previous_reward_item_id_);
    payload->previous_reward_gold =
        static_cast<uint32_t>(previous_reward_gold_);

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    previous_reward_claimed_ = 0;
    return 1;
}

// IDA_EA: 0x832A4C6
// IDA_NAME: _ZN19CCharacBloodDungeon8loadDataEP5CUserPc
// DEMANGLED: CCharacBloodDungeon::loadData(CUser *,char *)
bool CCharacBloodDungeon::loadData(CUser* user, char* data) {
    (void)user;
    const SIG_BLOOD_DUNGEON* const payload =
        reinterpret_cast<const SIG_BLOOD_DUNGEON*>(data);
    week_point_ = static_cast<int>(payload->week_point);
    week_enter_count_ = static_cast<int>(payload->week_enter_count);
    week_use_gold_ = static_cast<int>(payload->week_use_gold);
    last_play_date_ = payload->last_play_date;
    enter_count_ = static_cast<int>(payload->enter_count);
    previous_rank_ = static_cast<int>(payload->previous_rank);
    previous_reward_item_id_ =
        static_cast<int>(payload->previous_reward_item_id);
    previous_reward_gold_ = static_cast<int>(payload->previous_reward_gold);
    return true;
}

// IDA_EA: 0x832A53A
// IDA_NAME: _ZNK19CCharacBloodDungeon7getDataEPc
// DEMANGLED: CCharacBloodDungeon::getData(char *)const
void CCharacBloodDungeon::getData(char* data) const {
    SIG_BLOOD_DUNGEON* const payload =
        reinterpret_cast<SIG_BLOOD_DUNGEON*>(data);
    payload->week_point = static_cast<uint32_t>(week_point_);
    payload->week_enter_count = static_cast<uint32_t>(week_enter_count_);
    payload->week_use_gold = static_cast<uint32_t>(week_use_gold_);
    payload->last_play_date = last_play_date_;
    payload->enter_count = static_cast<uint32_t>(enter_count_);
    payload->previous_rank = static_cast<uint32_t>(previous_rank_);
    payload->previous_reward_claimed = previous_reward_claimed_;
    payload->previous_reward_item_id =
        static_cast<uint32_t>(previous_reward_item_id_);
    payload->previous_reward_gold =
        static_cast<uint32_t>(previous_reward_gold_);
}

// IDA_EA: 0x832A5B6
// IDA_NAME: _ZN19CCharacBloodDungeon10ResetDailyEv
// DEMANGLED: CCharacBloodDungeon::ResetDaily(void)
void CCharacBloodDungeon::ResetDaily() {
    enter_count_ = 0;
    time_t timer =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    tm local_tm;
    tm* const current = localtime_r(&timer, &local_tm);
    if (current->tm_wday == 1) {
        week_point_ = 0;
        week_enter_count_ = 0;
        week_use_gold_ = 0;
        previous_reward_claimed_ = 0;
    }
    current->tm_hour = 0;
    current->tm_min = 0;
    current->tm_sec = 0;
}

// IDA_EA: 0x832A716
// IDA_NAME: _ZN19CCharacBloodDungeonD2Ev
// DEMANGLED: CCharacBloodDungeon::~CCharacBloodDungeon()
// IDA_EA: 0x832A746
// IDA_NAME: _ZN19CCharacBloodDungeonD0Ev
// DEMANGLED: CCharacBloodDungeon::~CCharacBloodDungeon()
CCharacBloodDungeon::~CCharacBloodDungeon() {}

// IDA_EA: 0x832AF6E
// IDA_NAME: _ZN19CCharacBloodDungeonC2Ev
// DEMANGLED: CCharacBloodDungeon::CCharacBloodDungeon(void)
CCharacBloodDungeon::CCharacBloodDungeon() {}

// IDA_EA: 0x85BFE88
// IDA_NAME: _ZNK19CCharacBloodDungeon13GetEnterCountEv
// DEMANGLED: CCharacBloodDungeon::GetEnterCount(void)const
int CCharacBloodDungeon::GetEnterCount() const {
    return enter_count_;
}

// IDA_EA: 0x85BFE94
// IDA_NAME: _ZNK19CCharacBloodDungeon19GetWeeklyEnterCountEv
// DEMANGLED: CCharacBloodDungeon::GetWeeklyEnterCount(void)const
int CCharacBloodDungeon::GetWeeklyEnterCount() const {
    return week_enter_count_;
}

// IDA_EA: 0x85BFEA0
// IDA_NAME: _ZN19CCharacBloodDungeon13IncEnterCountEv
// DEMANGLED: CCharacBloodDungeon::IncEnterCount(void)
CCharacBloodDungeon* CCharacBloodDungeon::IncEnterCount() {
    alter();
    ++enter_count_;
    ++week_enter_count_;
    return this;
}

// IDA_EA: 0x85BFED2
// IDA_NAME: _ZN19CCharacBloodDungeon8IncPointEi
// DEMANGLED: CCharacBloodDungeon::IncPoint(int)
CCharacBloodDungeon* CCharacBloodDungeon::IncPoint(int value) {
    alter();
    week_point_ += value;
    return this;
}

// IDA_EA: 0x85BFEF6
// IDA_NAME: _ZN19CCharacBloodDungeon10IncUseGoldEi
// DEMANGLED: CCharacBloodDungeon::IncUseGold(int)
CCharacBloodDungeon* CCharacBloodDungeon::IncUseGold(int value) {
    alter();
    week_use_gold_ += value;
    return this;
}
