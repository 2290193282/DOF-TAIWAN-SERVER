#include "old_game/player/session/online_preliminary_member_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/online_preliminary_stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace online_preliminary {

// IDA_EA: 0x8568F02
// IDA_NAME: _ZNK18online_preliminary24COnlinePreliminaryMember9GetRatingEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::GetRating(void)const
float COnlinePreliminaryMember::GetRating() const {
    return rating_;
}

// IDA_EA: 0x8585F34
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMemberC2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::COnlinePreliminaryMember(void)
COnlinePreliminaryMember::COnlinePreliminaryMember() {}

// IDA_EA: 0x8585F50
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMemberD2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::~COnlinePreliminaryMember()
// IDA_EA: 0x8585F80
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMemberD0Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::~COnlinePreliminaryMember()
COnlinePreliminaryMember::~COnlinePreliminaryMember() {}

// IDA_EA: 0x8585F9E
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember8loadDataEP5CUserPc
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::loadData(CUser *,char *)
bool COnlinePreliminaryMember::loadData(CUser* user, char* data) {
    (void)user;
    const SIG_ONLINE_PRELIMINARY_MEMBER* payload =
        reinterpret_cast<const SIG_ONLINE_PRELIMINARY_MEMBER*>(data);
    rating_ = payload->rating;
    win_count_ = payload->win_count;
    lose_count_ = payload->lose_count;
    return 1;
}

// IDA_EA: 0x8585FD6
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember9_saveDataEP5CUser
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::_saveData(CUser *)
int COnlinePreliminaryMember::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "OnlinePreliminaryData.cpp", 33);
    CStreamGuard guard(stream, true);
    (*guard) << 0x1e7;
    (*guard) << user->GetUID();
    SIG_ONLINE_PRELIMINARY_MEMBER* payload =
        guard->GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>();
    payload->server_group =
        static_cast<uint16_t>(user->GetServerGroup());
    payload->charac_no = user->getCurCharacNo();
    payload->rating = rating_;
    payload->win_count = win_count_;
    payload->lose_count = lose_count_;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x8586100
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember6_resetEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::_reset(void)
int COnlinePreliminaryMember::_reset() {
    rating_ = 0.0f;
    win_count_ = 0;
    lose_count_ = 0;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x8586124
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember16IncreaseWinCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::IncreaseWinCount(void)
charac_expand::CData* COnlinePreliminaryMember::IncreaseWinCount() {
    ++win_count_;
    return alter();
}

// IDA_EA: 0x8586146
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember17IncreaseLoseCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::IncreaseLoseCount(void)
charac_expand::CData* COnlinePreliminaryMember::IncreaseLoseCount() {
    ++lose_count_;
    return alter();
}

// IDA_EA: 0x8586168
// IDA_NAME: _ZN18online_preliminary24COnlinePreliminaryMember9SetRatingEf
// DEMANGLED: online_preliminary::COnlinePreliminaryMember::SetRating(float)
charac_expand::CData* COnlinePreliminaryMember::SetRating(float rating) {
    rating_ = rating;
    return alter();
}

}  // namespace online_preliminary
