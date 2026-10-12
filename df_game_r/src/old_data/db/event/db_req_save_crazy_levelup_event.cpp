#include "old_data/db/event/db_req_save_crazy_levelup_event.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E55E
// IDA_NAME: _ZN27DB_ReqSaveCrazyLevelupEventC2Ev
// DEMANGLED: DB_ReqSaveCrazyLevelupEvent::DB_ReqSaveCrazyLevelupEvent(void)
DB_ReqSaveCrazyLevelupEvent::DB_ReqSaveCrazyLevelupEvent() : DBDispatcher() {}

// IDA_EA: 0x845EDD0
// IDA_NAME: _ZN27DB_ReqSaveCrazyLevelupEventD2Ev
// DEMANGLED: DB_ReqSaveCrazyLevelupEvent::~DB_ReqSaveCrazyLevelupEvent()
// IDA_EA: 0x845EE00
// IDA_NAME: _ZN27DB_ReqSaveCrazyLevelupEventD0Ev
// DEMANGLED: DB_ReqSaveCrazyLevelupEvent::~DB_ReqSaveCrazyLevelupEvent()
// deleting destructor: D0 calls this D2 body, then operator delete.
DB_ReqSaveCrazyLevelupEvent::~DB_ReqSaveCrazyLevelupEvent() {}

// IDA_EA: 0x842527A
// IDA_NAME: _ZN27DB_ReqSaveCrazyLevelupEvent8dispatchEiiP6Stream
// DEMANGLED: DB_ReqSaveCrazyLevelupEvent::dispatch(int,int,Stream *)
bool DB_ReqSaveCrazyLevelupEvent::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    // IDA calls this typed specialization directly at 0x842528D.  Its old
    // body checks enable_out(13), advances out_ptr(13), and returns the
    // physical payload pointer.  The dispatch itself does not null-check it.
    SIG_SAVE_CRAZY_LEVELUP_EVENT* payload =
        stream->GetOutBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    const int charac_no = static_cast<int>(payload->charac_no);
    const char* member_id = NumberToString(payload->m_id, 0);
    db_handle->set_query(
        "inSert into log_fighter_event(m_id,event_id,charac_no,occ_time,occ_cnt) values(%s,'%s',%d,now(),1)",
        member_id,
        payload->event_id,
        charac_no);

    if (db_handle->exec(true) != true) {
        // The old body retries only after the INSERT failed.  It re-forms the
        // NumberToString slot before issuing the UPDATE and returns false only
        // when that second execution fails.
        member_id = NumberToString(payload->m_id, 0);
        db_handle->set_query(
            "upDate log_fighter_event set occ_time=now(),occ_cnt=occ_cnt+1 where m_id=%s and event_id='%s' and charac_no=%d",
            member_id,
            payload->event_id,
            charac_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
