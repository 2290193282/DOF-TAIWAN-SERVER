#include "old_data/db/equipment_upgrade/db_req_item_upgrade_log.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>


namespace {

enum {
    kDbSignalReqItemUpgradeLog = 192,
    kItemUpgradeLogDbHandle = 4,
};

}  // namespace

// IDA_EA: 0x844E606
// IDA_NAME: _ZN20DB_ReqItemUpgradeLogC2Ev
// DEMANGLED: DB_ReqItemUpgradeLog::DB_ReqItemUpgradeLog(void)
DB_ReqItemUpgradeLog::DB_ReqItemUpgradeLog() {}

// IDA_EA: 0x845EBFC
// IDA_NAME: _ZN20DB_ReqItemUpgradeLogD2Ev
// DEMANGLED: DB_ReqItemUpgradeLog::~DB_ReqItemUpgradeLog()
// IDA_EA: 0x845EC2C
// IDA_NAME: _ZN20DB_ReqItemUpgradeLogD0Ev
// DEMANGLED: DB_ReqItemUpgradeLog::~DB_ReqItemUpgradeLog() deleting destructor
DB_ReqItemUpgradeLog::~DB_ReqItemUpgradeLog() {}

// IDA_EA: 0x8425DBE
// IDA_NAME: _ZN20DB_ReqItemUpgradeLog8dispatchEiiP6Stream
// DEMANGLED: DB_ReqItemUpgradeLog::dispatch(int,int,Stream *)
bool DB_ReqItemUpgradeLog::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    uint32_t server_id = 0;
    uint32_t item_id = 0;
    uint32_t charac_no = 0;
    int develop = 0;
    unsigned char success = 1;
    char charac_name[29];
    *stream >> server_id;
    *stream >> item_id;
    *stream >> charac_no;
    *stream >> develop;
    *stream >> success;
    *stream >> charac_name;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kItemUpgradeLogDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into log_item_develop(occ_time,it_no,server_id,charac_no,charac_name,develop,sucess_flag) values(now(),%d,%d,%d,'%s',%d,%d)",
        item_id,
        server_id,
        charac_no,
        charac_name,
        develop,
        success);
    return db_handle->exec(true);
}

// IDA_EA: 0x8425ECE
// IDA_NAME: _ZN20DB_ReqItemUpgradeLog11makeRequestEjjjPKcjb
// DEMANGLED: DB_ReqItemUpgradeLog::makeRequest(unsigned int,unsigned int,unsigned int,char const*,unsigned int,bool)
void DB_ReqItemUpgradeLog::makeRequest(unsigned int server_id,
                                       unsigned int item_id,
                                       unsigned int charac_no,
                                       const char* charac_name,
                                       unsigned int develop,
                                       bool success) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25591);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqItemUpgradeLog;
    *guard << -1;
    *guard << server_id;
    *guard << item_id;
    *guard << charac_no;
    *guard << static_cast<int>(develop);
    *guard << success;
    *guard << charac_name;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}
