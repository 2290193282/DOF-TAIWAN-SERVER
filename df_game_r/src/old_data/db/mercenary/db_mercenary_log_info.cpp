#include "old_data/db/mercenary/db_mercenary_log_info.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844FAEA
// IDA_NAME: _ZN19DB_MercenaryLogInfoC2Ev
// DEMANGLED: DB_MercenaryLogInfo::DB_MercenaryLogInfo(void)
DB_MercenaryLogInfo::DB_MercenaryLogInfo() {}

// IDA_EA: 0x845B39E
// IDA_NAME: _ZN19DB_MercenaryLogInfoD2Ev
// DEMANGLED: DB_MercenaryLogInfo::~DB_MercenaryLogInfo()
// IDA_EA: 0x845B3CE
// IDA_NAME: _ZN19DB_MercenaryLogInfoD0Ev
// DEMANGLED: DB_MercenaryLogInfo::~DB_MercenaryLogInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_MercenaryLogInfo::~DB_MercenaryLogInfo() {}

// IDA_EA: 0x8446196
// IDA_NAME: _ZN19DB_MercenaryLogInfo8dispatchEiiP6Stream
// DEMANGLED: DB_MercenaryLogInfo::dispatch(int,int,Stream *)
bool DB_MercenaryLogInfo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int m_id = 0;
    int charac_no = 0;
    int start_time = 0;
    int finish_time = 0;
    char type = 0;
    char period = 0;
    char area = 0;
    int gold = 0;
    int item_id = 0;
    int rarity = 0;

    *stream >> m_id;
    *stream >> charac_no;
    *stream >> start_time;
    *stream >> finish_time;
    *stream >> type;
    *stream >> period;
    *stream >> area;
    *stream >> gold;
    *stream >> item_id;
    *stream >> rarity;

    const char* account_id = NumberToString(m_id, 0);
    db_handle->set_query(
        "inSert into log_mercenary(m_id,charac_no,start_time,finish_time,type,period,area,gold,item_id,rarity) values(%s,%u,from_unixtime(%d),from_unixtime(%d),%d,%d,%d,%d,%d,%d)",
        account_id,
        charac_no,
        start_time,
        finish_time,
        static_cast<int>(type),
        static_cast<int>(period),
        static_cast<int>(area),
        gold,
        item_id,
        rarity);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8446340
// IDA_NAME: _ZN19DB_MercenaryLogInfo11makeRequestEjjjjccciii
// DEMANGLED: DB_MercenaryLogInfo::makeRequest(unsigned int,unsigned int,unsigned int,unsigned int,char,char,char,int,int,int)
void DB_MercenaryLogInfo::makeRequest(unsigned int m_id,
                                      unsigned int charac_no,
                                      unsigned int start_time,
                                      unsigned int finish_time,
                                      char type,
                                      char period,
                                      char area,
                                      int gold,
                                      int item_id,
                                      int rarity) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 44496);
    CStreamGuard guard(stream, true);
    *guard << 0x2B8;
    *guard << -1;
    *guard << m_id;
    *guard << charac_no;
    *guard << start_time;
    *guard << finish_time;
    *guard << type;
    *guard << period;
    *guard << area;
    *guard << gold;
    *guard << item_id;
    *guard << rarity;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}
