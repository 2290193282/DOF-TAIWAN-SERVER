#include "old_data/db/event/db_save_give_avenger_title_flag.h"

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
    kDbSignalSaveGiveAvengerTitleFlag = 0x28D,
};

}  // namespace

// IDA_EA: 0x844F8BA
// IDA_NAME: _ZN27DB_SaveGiveAvengerTitleFlagC2Ev
// DEMANGLED: DB_SaveGiveAvengerTitleFlag::DB_SaveGiveAvengerTitleFlag(void)
DB_SaveGiveAvengerTitleFlag::DB_SaveGiveAvengerTitleFlag() {}

// IDA_EA: 0x845BA52
// IDA_NAME: _ZN27DB_SaveGiveAvengerTitleFlagD2Ev
// DEMANGLED: DB_SaveGiveAvengerTitleFlag::~DB_SaveGiveAvengerTitleFlag()
// IDA_EA: 0x845BA82
// IDA_NAME: _ZN27DB_SaveGiveAvengerTitleFlagD0Ev
// DEMANGLED: DB_SaveGiveAvengerTitleFlag::~DB_SaveGiveAvengerTitleFlag() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGiveAvengerTitleFlag::~DB_SaveGiveAvengerTitleFlag() {}

// IDA_EA: 0x8442226
// IDA_NAME: _ZN27DB_SaveGiveAvengerTitleFlag8dispatchEiiP6Stream
// DEMANGLED: DB_SaveGiveAvengerTitleFlag::dispatch(int,int,Stream *)
bool DB_SaveGiveAvengerTitleFlag::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    *stream >> account_id;
    *stream >> charac_no;

    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "inSert into event_1107_avenger_plan(m_id, charac_no, occ_date) values(%s, %u, now())",
        account_id_text,
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84422DC
// IDA_NAME: _ZN27DB_SaveGiveAvengerTitleFlag11makeRequestEijj
// DEMANGLED: DB_SaveGiveAvengerTitleFlag::makeRequest(int,unsigned int,unsigned int)
void DB_SaveGiveAvengerTitleFlag::makeRequest(int user_idx,
                                              unsigned int account_id,
                                              unsigned int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 41590);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSaveGiveAvengerTitleFlag;
    *guard << user_idx;
    *guard << account_id;
    *guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
