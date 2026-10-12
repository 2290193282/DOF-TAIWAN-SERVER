#include "old_data/db/auth_security/db_passpad_update_fail_cnt.h"

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
#include <string.h>
#include <time.h>



#pragma pack(push, 1)
struct SIG_PASSPAD_UPDATE_FAIL_CNT {
    uint32_t account_id;   // +0
    uint32_t fail_count;   // +4
    char ip[16];           // +8
};
#pragma pack(pop)

typedef char SIG_PASSPAD_UPDATE_FAIL_CNT_Size_Must_Be_24[
    (sizeof(SIG_PASSPAD_UPDATE_FAIL_CNT) == 24) ? 1 : -1];

// IDA_EA: 0x844E4D2
// IDA_NAME: _ZN23DB_PassPadUpdateFailCntC2Ev
// DEMANGLED: DB_PassPadUpdateFailCnt::DB_PassPadUpdateFailCnt(void)
DB_PassPadUpdateFailCnt::DB_PassPadUpdateFailCnt() {}

// IDA_EA: 0x845EFA4
// IDA_NAME: _ZN23DB_PassPadUpdateFailCntD2Ev
// DEMANGLED: DB_PassPadUpdateFailCnt::~DB_PassPadUpdateFailCnt()
// IDA_EA: 0x845EFD4
// IDA_NAME: _ZN23DB_PassPadUpdateFailCntD0Ev
// DEMANGLED: DB_PassPadUpdateFailCnt::~DB_PassPadUpdateFailCnt() deleting destructor
DB_PassPadUpdateFailCnt::~DB_PassPadUpdateFailCnt() {}

// IDA_EA: 0x8423BC4
// IDA_NAME: _ZN23DB_PassPadUpdateFailCnt8dispatchEiiP6Stream
// DEMANGLED: DB_PassPadUpdateFailCnt::dispatch(int,int,Stream *)
bool DB_PassPadUpdateFailCnt::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* mouse_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_PASSPAD_UPDATE_FAIL_CNT payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    mouse_db->set_query(
        "upDate member_mousepass set fail_cnt=%d where m_id=%s and enable_flag = '1'",
        payload.fail_count,
        NumberToString(payload.account_id, 0));
    mouse_db->exec(true);

    if (payload.fail_count) {
        char previous_password[36];
        memset(previous_password, 0, sizeof(previous_password));
        mouse_db->set_query("seLect mousepass from member_mousepass where m_id=%s",
                            NumberToString(payload.account_id, 0));
        if (mouse_db->exec(true) != true || !mouse_db->get_n_rows() ||
            mouse_db->fetch() != true) {
            return false;
        }
        if (mouse_db->get_str(0, previous_password, 36) != true) {
            return false;
        }

        char month[10];
        memset(month, 0, sizeof(month));
        time_t timer = GlobalData::system_time()
                           ? GlobalData::system_time()->getCurSec()
                           : time(0);
        struct tm tp;
        localtime_r(&timer, &tp);
        strftime(month, sizeof(month), "%Y%m", &tp);

        mouse_db->set_query(
            "inSert into member_mousepass_history_%s (occ_time, m_id, pre_mousepass, modify_type, ip_info, port_info) values (now(),%s,'%s',%d,'%s','')",
            month,
            NumberToString(payload.account_id, 0),
            previous_password,
            17,
            payload.ip);
        mouse_db->exec(true);
    }

    MySQL* member_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    member_db->set_query(
        "upDate member_security_grade set goblin_fail_cnt=%d where m_id=%s",
        payload.fail_count,
        NumberToString(payload.account_id, 0));
    bool need_insert = true;
    if (member_db->exec(true) == true && member_db->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        member_db->set_query(
            "inSert into member_security_grade(m_id, goblin_fail_cnt) values(%s, %d)",
            NumberToString(payload.account_id, 0),
            payload.fail_count);
        if (member_db->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8423EB4
// IDA_NAME: _ZN23DB_PassPadUpdateFailCnt11makeRequestEjjPc
// DEMANGLED: DB_PassPadUpdateFailCnt::makeRequest(unsigned int,unsigned int,char *)
void DB_PassPadUpdateFailCnt::makeRequest(unsigned int account_id,
                                          unsigned int fail_count,
                                          char* ip) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 24369);
    CStreamGuard guard(stream, true);
    *guard << 164;
    *guard << -1;

    SIG_PASSPAD_UPDATE_FAIL_CNT* payload =
        static_cast<SIG_PASSPAD_UPDATE_FAIL_CNT*>(
            guard->GetInBufferUntyped(sizeof(SIG_PASSPAD_UPDATE_FAIL_CNT)));
    payload->account_id = account_id;
    payload->fail_count = fail_count;
    strncpy(payload->ip, ip, 0x10u);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
