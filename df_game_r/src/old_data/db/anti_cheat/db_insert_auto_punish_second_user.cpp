#include "old_data/db/anti_cheat/db_insert_auto_punish_second_user.h"

#include "old_data/db/anti_cheat/auto_punish_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdio.h>
#include <string.h>


namespace {

enum {
    kDbSignalInsertAutoPunishSecondUser = 267,
    kAutoPunishDbHandle = 6,
    kMemberDbHandle = 1,
};

}  // namespace

// IDA_EA: 0x844EA66
// IDA_NAME: _ZN29DB_InsertAutoPunishSecondUserC2Ev
// DEMANGLED: DB_InsertAutoPunishSecondUser::DB_InsertAutoPunishSecondUser(void)
DB_InsertAutoPunishSecondUser::DB_InsertAutoPunishSecondUser() {}

// IDA_EA: 0x845E01A
// IDA_NAME: _ZN29DB_InsertAutoPunishSecondUserD2Ev
// DEMANGLED: DB_InsertAutoPunishSecondUser::~DB_InsertAutoPunishSecondUser()
// IDA_EA: 0x845E04A
// IDA_NAME: _ZN29DB_InsertAutoPunishSecondUserD0Ev
// DEMANGLED: DB_InsertAutoPunishSecondUser::~DB_InsertAutoPunishSecondUser()
DB_InsertAutoPunishSecondUser::~DB_InsertAutoPunishSecondUser() {}

// IDA_EA: 0x842DEB4
// IDA_NAME: _ZN29DB_InsertAutoPunishSecondUser8dispatchEiiP6Stream
// DEMANGLED: DB_InsertAutoPunishSecondUser::dispatch(int,int,Stream *)
bool DB_InsertAutoPunishSecondUser::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* auto_db =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoPunishDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const SIG_INSERT_AUTO_PUNISH_SECOND_USER* request =
        stream->GetOutBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>();
    const unsigned long long trade_gold =
        (static_cast<unsigned long long>(request->trade_gold_high) << 32) |
        request->trade_gold_low;
    const int punish_flag = request->punish ? 1 : 0;

    // The old 32-bit image passes each uint64 as two cdecl dwords even though
    // these historical SQL format strings use %d. Keep that exact ABI.
    auto_db->set_query(
        "upDate auto_punish_second_user set occ_time=now(),total_trade_cnt=total_trade_cnt+1,trade_cnt=trade_cnt+1,total_trade_gold=total_trade_gold+%d,trade_gold=trade_gold+%d,punish_flag=%d where m_id=%s",
        trade_gold,
        trade_gold,
        punish_flag,
        NumberToString(request->account_id, 0));
    if (auto_db->exec(true) != true || !auto_db->getAffectedRowCount()) {
        auto_db->set_query(
            "inSert into auto_punish_second_user(m_id,occ_time,total_trade_cnt,trade_cnt,total_trade_gold,trade_gold,punish_flag) values(%s,now(),1,1,%d,%d,%d)",
            NumberToString(request->account_id, 0),
            trade_gold,
            trade_gold,
            punish_flag);
        auto_db->exec(true);
    }

    auto_db->set_query(
        "upDate auto_punish_second_log set occ_time=now(),trade_cnt=trade_cnt+1,trade_gold=trade_gold+%d where m_id=%s and hack_m_id=%s",
        trade_gold,
        NumberToString(request->account_id, 0),
        NumberToString(request->hack_account_id, 1));
    if (auto_db->exec(true) != true || !auto_db->getAffectedRowCount()) {
        auto_db->set_query(
            "inSert into auto_punish_second_log(m_id,hack_m_id,occ_time,trade_cnt,trade_gold) values(%s,%s,now(),1,%d)",
            NumberToString(request->account_id, 0),
            NumberToString(request->hack_account_id, 1),
            trade_gold);
        auto_db->exec(true);
    }

    if (!request->punish) {
        return true;
    }

    MySQL* member_db =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kMemberDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const int apply_flag = request->punish ? 2 : 0;
    member_db->set_query(
        "upDate member_punish_info set occ_time=now(), start_time=now(), end_time='9999-12-31 23:59:59', apply_flag=%d where m_id=%d and punish_type=%d",
        apply_flag,
        request->account_id,
        4);
    if (member_db->exec(true) != true ||
        !member_db->getAffectedRowCount()) {
        member_db->set_query(
            "inSert into member_punish_info(m_id, punish_type, occ_time, start_time, end_time, punish_value, apply_flag) values(%s, %d, now(), now(), '9999-12-31 23:59:59', 0, %d)",
            NumberToString(request->account_id, 0),
            4,
            apply_flag);
        if (member_db->exec(true) != true) {
            return false;
        }
    }

    char reason[255];
    memset(reason, 0, sizeof(reason));
    sprintf(reason,
            "{Auto 2nd Information-Mail}|*|ARS 2nd Punish (Trade) / %s / %d",
            NumberToString(request->hack_account_id, 0),
            trade_gold);
    member_db->set_query(
        "inSert into member_punish_info_history(m_id, punish_type, occ_time, punish_value, start_time, end_time, admin_id, apply_flag, reason) values(%s, 4, now(), 6, now(), '9999-12-31 23:59:59', 'ARS', 2, '%s')",
        NumberToString(request->account_id, 0),
        reason);
    return member_db->exec(true) == true;
}

// IDA_EA: 0x842E34E
// IDA_NAME: _ZN29DB_InsertAutoPunishSecondUser11makeRequestEjjyb
// DEMANGLED: DB_InsertAutoPunishSecondUser::makeRequest(unsigned int, unsigned int, unsigned long long, bool)
void DB_InsertAutoPunishSecondUser::makeRequest(
    unsigned int account_id,
    unsigned int hack_account_id,
    unsigned long long trade_gold,
    bool punish) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 29428);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertAutoPunishSecondUser;
    *guard << -1;
    SIG_INSERT_AUTO_PUNISH_SECOND_USER* request =
        guard->GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>();
    request->account_id = account_id;
    request->hack_account_id = hack_account_id;
    request->trade_gold_low = static_cast<uint32_t>(trade_gold);
    request->trade_gold_high = static_cast<uint32_t>(trade_gold >> 32);
    request->punish = punish ? 1 : 0;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
