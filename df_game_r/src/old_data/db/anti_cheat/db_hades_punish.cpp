#include "old_data/db/anti_cheat/db_hades_punish.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>



// IDA_EA: 0x844FAB2
// IDA_NAME: _ZN14DB_HadesPunishC2Ev
// DEMANGLED: DB_HadesPunish::DB_HadesPunish(void)
DB_HadesPunish::DB_HadesPunish() {}

// IDA_EA: 0x845B43A
// IDA_NAME: _ZN14DB_HadesPunishD2Ev
// DEMANGLED: DB_HadesPunish::~DB_HadesPunish()
// IDA_EA: 0x845B46A
// IDA_NAME: _ZN14DB_HadesPunishD0Ev
// DEMANGLED: DB_HadesPunish::~DB_HadesPunish() deleting destructor
DB_HadesPunish::~DB_HadesPunish() {}

// IDA_EA: 0x8445AA6
// IDA_NAME: _ZN14DB_HadesPunish8dispatchEiiP6Stream
// DEMANGLED: DB_HadesPunish::dispatch(int,int,Stream *)
bool DB_HadesPunish::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_HADES_PUNISH payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    payload.reason[sizeof(payload.reason) - 1] = '\0';

    char end_time[100];
    if (payload.punish_days) {
        snprintf(end_time,
                 sizeof(end_time),
                 "adddate(now(),interval %d day)",
                 payload.punish_days);
    } else {
        memcpy(end_time, "'9999-12-31 23:59:59'", 0x16u);
        end_time[0x16] = 0;
    }
    const int apply_flag = payload.apply_switch ? 2 : 1;
    const int punish_type = (payload.punish_kind == 1) ? 3 : 12;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate member_punish_info set occ_time=now(), start_time=now(), end_time=%s, apply_flag=%d where m_id=%s and punish_type=%d",
        end_time,
        apply_flag,
        NumberToString(payload.account_id, 0),
        punish_type);
    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        db_handle->set_query(
            "inSert into member_punish_info (m_id, punish_type, occ_time, start_time, end_time, punish_value, apply_flag) values(%s, %d, now(), now(), %s, 0, %d)",
            NumberToString(payload.account_id, 0),
            punish_type,
            end_time,
            apply_flag);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    char year[10];
    memset(year, 0, sizeof(year));
    time_t cur_sec = GlobalData::system_time()
                         ? GlobalData::system_time()->getCurSec()
                         : time(0);
    struct tm tp;
    localtime_r(&cur_sec, &tp);
    strftime(year, sizeof(year), "%Y", &tp);

    const char* reason = payload.reason[0]
                             ? payload.reason
                             : ((payload.punish_kind == 1)
                                    ? "HADES trade punish"
                                    : "HADES hack punish");
    db_handle->set_query(
        "inSert into member_punish_info_history_%s (m_id, punish_type, occ_time, punish_value, start_time, end_time, admin_id, apply_flag, reason) values(%s, %d, now(), 6, now(), %s, 'HADES', %d, '%s') ",
        year,
        NumberToString(payload.account_id, 0),
        punish_type,
        end_time,
        apply_flag,
        reason);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (punish_type == 3 || (punish_type == 12 && apply_flag == 1)) {
        db_handle->set_query(
            "inSert into member_punish_info_ars_mail (m_id, punish_type, apply_flag, occ_time) values(%s, %d, %d, now())",
            NumberToString(payload.account_id, 0),
            punish_type,
            apply_flag);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8445DD4
// IDA_NAME: _ZN14DB_HadesPunish11makeRequestEjjjjjPc
// DEMANGLED: DB_HadesPunish::makeRequest(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,char *)
void DB_HadesPunish::makeRequest(unsigned int account_id,
                                 unsigned int punish_kind,
                                 unsigned int punish_days,
                                 unsigned int apply_switch,
                                 unsigned int reserved,
                                 char* reason) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 44377);
    CStreamGuard guard(stream, true);
    *guard << 694;
    *guard << -1;

    SIG_HADES_PUNISH* payload =
        static_cast<SIG_HADES_PUNISH*>(
            guard->GetInBufferUntyped(sizeof(SIG_HADES_PUNISH)));
    memset(payload, 0, sizeof(SIG_HADES_PUNISH));
    payload->account_id = account_id;
    payload->punish_kind = punish_kind;
    payload->punish_days = punish_days;
    payload->apply_switch = apply_switch;
    payload->reserved = reserved;
    if (reason) {
        strncpy(payload->reason, reason, sizeof(payload->reason) - 1);
        payload->reason[sizeof(payload->reason) - 1] = '\0';
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
