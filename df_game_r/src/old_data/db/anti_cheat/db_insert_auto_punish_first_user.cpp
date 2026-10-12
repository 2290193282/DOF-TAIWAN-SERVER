#include "old_data/db/anti_cheat/db_insert_auto_punish_first_user.h"

#include "old_data/db/anti_cheat/auto_punish_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdio.h>
#include <string.h>
#include <time.h>


namespace {

enum {
    kDbSignalInsertAutoPunishFirstUser = 266,
    kAutoPunishDbHandle = 6,
    kMemberDbHandle = 1,
};

}  // namespace

// IDA_EA: 0x844EA4A
// IDA_NAME: _ZN28DB_InsertAutoPunishFirstUserC2Ev
// DEMANGLED: DB_InsertAutoPunishFirstUser::DB_InsertAutoPunishFirstUser(void)
DB_InsertAutoPunishFirstUser::DB_InsertAutoPunishFirstUser() {}

// IDA_EA: 0x845E068
// IDA_NAME: _ZN28DB_InsertAutoPunishFirstUserD2Ev
// DEMANGLED: DB_InsertAutoPunishFirstUser::~DB_InsertAutoPunishFirstUser()
// IDA_EA: 0x845E098
// IDA_NAME: _ZN28DB_InsertAutoPunishFirstUserD0Ev
// DEMANGLED: DB_InsertAutoPunishFirstUser::~DB_InsertAutoPunishFirstUser()
DB_InsertAutoPunishFirstUser::~DB_InsertAutoPunishFirstUser() {}

// IDA_EA: 0x842D6CE
// IDA_NAME: _ZN28DB_InsertAutoPunishFirstUser8dispatchEiiP6Stream
// DEMANGLED: DB_InsertAutoPunishFirstUser::dispatch(int,int,Stream *)
bool DB_InsertAutoPunishFirstUser::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;
    (void)user_idx;

    const SIG_INSERT_AUTO_PUNISH_FIRST_USER* request =
        stream->GetOutBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>();
    MySQL* auto_db =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoPunishDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    auto_db->set_query(
        "inSert into auto_punish_first_user(m_id,occ_time,ip,hack_type,cnt,hack_sub_type,hack_sub_cnt,punish_flag) values(%s,now(),'%s',%d,%d,%d,%d,%d)",
        NumberToString(request->account_id, 0),
        request->ip,
        request->hack_type,
        request->count,
        request->hack_sub_type,
        request->hack_sub_count,
        request->punish_flag);
    if (auto_db->exec(true) != true) {
        auto_db->set_query(
            "upDate auto_punish_first_user set occ_time=now(),ip='%s',cnt=%d, hack_sub_cnt=%d, punish_flag=%d where m_id=%s and hack_type=%d and hack_sub_type=%d",
            request->ip,
            request->count,
            request->hack_sub_count,
            request->punish_flag,
            NumberToString(request->account_id, 0),
            request->hack_type,
            request->hack_sub_type);
        if (auto_db->exec(true) != true) {
            cMyTrace trace(
                "virtual bool DB_InsertAutoPunishFirstUser::dispatch(int, int, Stream*)",
                28923,
                0);
            trace("upDate auto_punish_first_user Fail");
        }
    }

    MySQL* member_db =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kMemberDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    switch (request->punish_flag) {
        case 2:
        case 7:
        case 8:
            return true;
        default:
            break;
    }

    int punish_type = 3;
    if (request->punish_flag == 10) {
        punish_type = 12;
    }
    const unsigned int punish_days = request->punish_value / 10;
    const unsigned int apply_switch = request->punish_value % 10;
    const int apply_flag = apply_switch ? 1 : 2;
    printf("[ON!!] request->etcValue_ : %d , dwPunishDay(%d), dwApplyFlag(%d)\n",
           request->punish_value,
           punish_days,
           apply_switch);

    if (punish_days) {
        member_db->set_query(
            "upDate member_punish_info set occ_time=now(), start_time=now(), end_time= adddate(now(),interval %d day), apply_flag=%d where m_id=%s and punish_type=%d",
            punish_days,
            apply_flag,
            NumberToString(request->account_id, 0),
            punish_type);
    } else {
        member_db->set_query(
            "upDate member_punish_info set occ_time=now(), start_time=now(), end_time='9999-12-31 23:59:59', apply_flag=%d where m_id=%s and punish_type=%d",
            apply_flag,
            NumberToString(request->account_id, 0),
            punish_type);
    }

    if (member_db->exec(true) != true ||
        !member_db->getAffectedRowCount()) {
        if (punish_days) {
            member_db->set_query(
                "inSert into member_punish_info(m_id, punish_type, occ_time, start_time, end_time, punish_value, apply_flag) values(%s, %d, now(), now(), adddate(now(),interval %d day), 0, %d)",
                NumberToString(request->account_id, 0),
                punish_type,
                punish_days,
                apply_flag);
        } else {
            member_db->set_query(
                "inSert into member_punish_info(m_id, punish_type, occ_time, start_time, end_time, punish_value, apply_flag) values(%s, %d, now(), now(), '9999-12-31 23:59:59', 0, %d)",
                NumberToString(request->account_id, 0),
                punish_type,
                apply_flag);
        }
        if (member_db->exec(true) != true) {
            return false;
        }
    }

    char reason[255];
    memset(reason, 0, sizeof(reason));
    sprintf(reason,
            "{Auto 1st Information-Mail}|*|ARS 1st Punish / %d / %d",
            request->hack_type,
            request->hack_sub_type);

    char year[10];
    memset(year, 0, sizeof(year));
    const time_t current_time = GlobalData::s_systemTime_.getCurSec();
    struct tm time_info;
    localtime_r(&current_time, &time_info);
    strftime(year, sizeof(year), "%Y", &time_info);

    if (punish_days) {
        member_db->set_query(
            "inSert into member_punish_info_history_%s(m_id, punish_type, occ_time, punish_value, start_time, end_time, admin_id, apply_flag, reason) values(%s, %d, now(), 6, now(), adddate(now(),interval %d day), 'ARS', %d, '%s')",
            year,
            NumberToString(request->account_id, 0),
            punish_type,
            punish_days,
            apply_flag,
            reason);
    } else {
        member_db->set_query(
            "inSert into member_punish_info_history_%s(m_id, punish_type, occ_time, punish_value, start_time, end_time, admin_id, apply_flag, reason) values(%s, %d, now(), 6, now(), '9999-12-31 23:59:59', 'ARS', %d, '%s')",
            year,
            NumberToString(request->account_id, 0),
            punish_type,
            apply_flag,
            reason);
    }
    if (member_db->exec(true) != true) {
        return false;
    }
    if (punish_type != 3 && (punish_type != 4 || apply_switch)) {
        return true;
    }

    member_db->set_query(
        "inSert into member_punish_info_ars_mail (m_id, punish_type, apply_flag, occ_time) values(%s, %d, %d, now())",
        NumberToString(request->account_id, 0),
        punish_type,
        apply_flag);
    return member_db->exec(true) == true;
}

// IDA_EA: 0x842DD7C
// IDA_NAME: _ZN28DB_InsertAutoPunishFirstUser11makeRequestEjPKcjjjjtj
// DEMANGLED: DB_InsertAutoPunishFirstUser::makeRequest(unsigned int, char const*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned short, unsigned int)
void DB_InsertAutoPunishFirstUser::makeRequest(
    unsigned int account_id,
    const char* ip,
    unsigned int hack_type,
    unsigned int count,
    unsigned int hack_sub_type,
    unsigned int hack_sub_count,
    unsigned short punish_flag,
    unsigned int punish_value) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 29246);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertAutoPunishFirstUser;
    *guard << -1;
    SIG_INSERT_AUTO_PUNISH_FIRST_USER* request =
        guard->GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>();
    request->account_id = account_id;
    strncpy(request->ip, ip, sizeof(request->ip));
    request->hack_type = hack_type;
    request->count = count;
    request->hack_sub_type = hack_sub_type;
    request->hack_sub_count = hack_sub_count;
    request->punish_flag = punish_flag;
    request->punish_value = punish_value;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
