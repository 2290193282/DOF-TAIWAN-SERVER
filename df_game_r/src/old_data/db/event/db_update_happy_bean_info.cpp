#include "old_data/db/event/db_update_happy_bean_info.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>


namespace {

enum {
    kDbSignalUpdateHappyBeanInfo = 187,
    kHappyBeanDbHandle = 1,
};

}  // namespace

// IDA_EA: 0x844E5B2
// IDA_NAME: _ZN22DB_UpdateHappyBeanInfoC2Ev
// DEMANGLED: DB_UpdateHappyBeanInfo::DB_UpdateHappyBeanInfo(void)
DB_UpdateHappyBeanInfo::DB_UpdateHappyBeanInfo() {}

// IDA_EA: 0x845ECE6
// IDA_NAME: _ZN22DB_UpdateHappyBeanInfoD2Ev
// DEMANGLED: DB_UpdateHappyBeanInfo::~DB_UpdateHappyBeanInfo()
// IDA_EA: 0x845ED16
// IDA_NAME: _ZN22DB_UpdateHappyBeanInfoD0Ev
// DEMANGLED: DB_UpdateHappyBeanInfo::~DB_UpdateHappyBeanInfo() deleting destructor
DB_UpdateHappyBeanInfo::~DB_UpdateHappyBeanInfo() {}

// IDA_EA: 0x84258EE
// IDA_NAME: _ZN22DB_UpdateHappyBeanInfo8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateHappyBeanInfo::dispatch(int,int,Stream *)
bool DB_UpdateHappyBeanInfo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kHappyBeanDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    uint32_t member_id = 0;
    uint32_t event_value = 0;
    char character_name[29];
    *stream >> member_id;
    *stream >> event_value;
    *stream >> character_name;

    db_handle->set_query(
        "upDate event_happybean_log set cnt=cnt+1 where occ_date=now() and m_id=%s",
        NumberToString(member_id, 0));
    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into event_happybean_log(occ_date,m_id,cnt) values(now(),%s,1)",
            NumberToString(member_id, 0));
        db_handle->exec(true);
    }
    return true;
}

// IDA_EA: 0x8425A02
// IDA_NAME: _ZN22DB_UpdateHappyBeanInfo11makeRequestEjjjPKc
// DEMANGLED: DB_UpdateHappyBeanInfo::makeRequest(unsigned int,unsigned int,unsigned int,char const*)
void DB_UpdateHappyBeanInfo::makeRequest(unsigned int user_idx,
                                         unsigned int member_id,
                                         unsigned int event_value,
                                         const char* character_name) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25507);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateHappyBeanInfo;
    *guard << user_idx;
    *guard << member_id;
    *guard << event_value;
    *guard << const_cast<char*>(character_name);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
