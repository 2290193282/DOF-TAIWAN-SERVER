#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x86521EC
// IDA_NAME: _ZN5CUser24UpdateBreakAwayLuckPointEv
// DEMANGLED: CUser::UpdateBreakAwayLuckPoint(void)
void CUser::UpdateBreakAwayLuckPoint() {
    setUsedBreakAwayLuckPoint(false);

    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 7154);
    CStreamGuard guard(stream, true);
    *guard << 329;
    *guard << this->GetUID();
    *guard << this->get_acc_id();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x86522EA
// IDA_NAME: _ZN5CUser30UpdateBreakAwayDungeonClearCntEv
// DEMANGLED: CUser::UpdateBreakAwayDungeonClearCnt(void)
void CUser::UpdateBreakAwayDungeonClearCnt() {
    setBreakAwayDungeonClear(false);

    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 7169);
    CStreamGuard guard(stream, true);
    *guard << 325;
    *guard << this->GetUID();
    *guard << this->get_acc_id();
    *guard << getBreakAwayDungeonClearCnt();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x865240C
// IDA_NAME: _ZN5CUser21UpdateCharacLinkBonusEjjj28ENUM_CHARAC_LINK_BONUS_STATE
// DEMANGLED: CUser::UpdateCharacLinkBonus(unsigned int,unsigned int,unsigned int,ENUM_CHARAC_LINK_BONUS_STATE)
void CUser::UpdateCharacLinkBonus(unsigned int exp,
                                 unsigned int gold,
                                 unsigned int linked_charac_no,
                                 ENUM_CHARAC_LINK_BONUS_STATE state) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 7185);
    CStreamGuard guard(stream, true);
    *guard << 332;
    *guard << this->GetUID();
    *guard << linked_charac_no;
    *guard << exp;
    *guard << gold;
    *guard << static_cast<int>(state);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);

    this->setCharacLinkBonusExp(0);
    this->setCharacLinkBonusGold(0);
}
