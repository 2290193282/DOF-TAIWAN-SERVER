#include "old_data/db/auth_security/db_security_card_update_fail_cnt.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



extern unsigned int generateSecurityCardQuestion(char* out)
    __asm__("_Z28generateSecurityCardQuestionPc");

#pragma pack(push, 1)
struct SIG_SECURITY_CARD_UPDATE_FAIL_CNT {
    uint32_t account_id;  // +0
    uint32_t count;       // +4
};
#pragma pack(pop)

typedef char SIG_SECURITY_CARD_UPDATE_FAIL_CNT_Size_Must_Be_8[
    (sizeof(SIG_SECURITY_CARD_UPDATE_FAIL_CNT) == 8) ? 1 : -1];

// IDA_EA: 0x844E932
// IDA_NAME: _ZN28DB_SecurityCardUpdateFailCntC2Ev
// DEMANGLED: DB_SecurityCardUpdateFailCnt::DB_SecurityCardUpdateFailCnt(void)
DB_SecurityCardUpdateFailCnt::DB_SecurityCardUpdateFailCnt() {}

// IDA_EA: 0x845E326
// IDA_NAME: _ZN28DB_SecurityCardUpdateFailCntD2Ev
// DEMANGLED: DB_SecurityCardUpdateFailCnt::~DB_SecurityCardUpdateFailCnt()
// IDA_EA: 0x845E356
// IDA_NAME: _ZN28DB_SecurityCardUpdateFailCntD0Ev
// DEMANGLED: DB_SecurityCardUpdateFailCnt::~DB_SecurityCardUpdateFailCnt() deleting destructor
DB_SecurityCardUpdateFailCnt::~DB_SecurityCardUpdateFailCnt() {}

// IDA_EA: 0x842BDDA
// IDA_NAME: _ZN28DB_SecurityCardUpdateFailCnt8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardUpdateFailCnt::dispatch(int,int,Stream *)
bool DB_SecurityCardUpdateFailCnt::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SECURITY_CARD_UPDATE_FAIL_CNT payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload.count) {
        db_handle->set_query(
            "upDate member_security_card set fail_cnt=%d where m_id=%s and apply_flag=1",
            payload.count,
            NumberToString(payload.account_id, 0));
    } else {
        char q_pos[5];
        memset(q_pos, 0, sizeof(q_pos));
        generateSecurityCardQuestion(q_pos);
        db_handle->set_query(
            "upDate member_security_card set fail_cnt=%d , q_pos = '%s' where m_id=%s and apply_flag=1",
            payload.count,
            q_pos,
            NumberToString(payload.account_id, 0));
    }
    db_handle->exec(true);

    MySQL* member_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    member_db->set_query(
        "upDate member_security_grade set security_card_fail_cnt=%d where m_id=%s",
        payload.count,
        NumberToString(payload.account_id, 0));
    bool need_insert = true;
    if (member_db->exec(true) == true && member_db->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        member_db->set_query(
            "inSert into member_security_grade(m_id, security_card_fail_cnt) values(%s, %d)",
            NumberToString(payload.account_id, 0),
            payload.count);
        if (member_db->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x842BFAA
// IDA_NAME: _ZN28DB_SecurityCardUpdateFailCnt11makeRequestEjj
// DEMANGLED: DB_SecurityCardUpdateFailCnt::makeRequest(unsigned int,unsigned int)
void DB_SecurityCardUpdateFailCnt::makeRequest(unsigned int account_id,
                                               unsigned int fail_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28427);
    CStreamGuard guard(stream, true);
    *guard << 257;
    *guard << -1;

    SIG_SECURITY_CARD_UPDATE_FAIL_CNT* payload =
        static_cast<SIG_SECURITY_CARD_UPDATE_FAIL_CNT*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_SECURITY_CARD_UPDATE_FAIL_CNT)));
    payload->account_id = account_id;
    payload->count = fail_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
