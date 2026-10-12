#include "old_data/db/event/db_event_account_character_level_up_once_gift.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalEventAccountCharacterLevelUpOnceGift = 0x319,
};

}  // namespace

// IDA_EA: 0x844FE32
// IDA_NAME: _ZN39DB_EventAccountCharacterLevelUpOnceGiftC2Ev
// DEMANGLED: DB_EventAccountCharacterLevelUpOnceGift::DB_EventAccountCharacterLevelUpOnceGift(void)
DB_EventAccountCharacterLevelUpOnceGift::
    DB_EventAccountCharacterLevelUpOnceGift() {}

// IDA_EA: 0x845AC9C
// IDA_NAME: _ZN39DB_EventAccountCharacterLevelUpOnceGiftD2Ev
// DEMANGLED: DB_EventAccountCharacterLevelUpOnceGift::~DB_EventAccountCharacterLevelUpOnceGift()
// IDA_EA: 0x845ACCC
// IDA_NAME: _ZN39DB_EventAccountCharacterLevelUpOnceGiftD0Ev
// DEMANGLED: DB_EventAccountCharacterLevelUpOnceGift::~DB_EventAccountCharacterLevelUpOnceGift() deleting destructor
DB_EventAccountCharacterLevelUpOnceGift::
    ~DB_EventAccountCharacterLevelUpOnceGift() {}

// IDA_EA: 0x8449E3E
// IDA_NAME: _ZN39DB_EventAccountCharacterLevelUpOnceGift8dispatchEiiP6Stream
// DEMANGLED: DB_EventAccountCharacterLevelUpOnceGift::dispatch(int,int,Stream *)
bool DB_EventAccountCharacterLevelUpOnceGift::dispatch(int signal,
                                                       int user_idx,
                                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    short server_id = 0;
    int charac_no = 0;
    short charac_job = 0;
    short charac_level = 0;
    *stream >> account_id;
    *stream >> server_id;
    *stream >> charac_no;
    *stream >> charac_job;
    *stream >> charac_level;

    db_handle->set_query(
        "inSert into event_1208_level_achieve(m_id ,server_id, charac_no, job_type, level, occ_date) values(%s, %d, %d, %d, %d, now())",
        NumberToString(account_id, 0),
        static_cast<int>(server_id),
        charac_no,
        static_cast<int>(charac_job),
        static_cast<int>(charac_level));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8449F66
// IDA_NAME: _ZN39DB_EventAccountCharacterLevelUpOnceGift11makeRequestEjsiss
// DEMANGLED: DB_EventAccountCharacterLevelUpOnceGift::makeRequest(unsigned int,short,int,short,short)
void DB_EventAccountCharacterLevelUpOnceGift::makeRequest(
    unsigned int account_id,
    short server_id,
    int charac_no,
    short charac_job,
    short charac_level) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47577);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalEventAccountCharacterLevelUpOnceGift;
    *guard << -1;
    *guard << account_id;
    *guard << server_id;
    *guard << charac_no;
    *guard << charac_job;
    *guard << charac_level;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
