#include "old_data/db/character/db_update_gm_data.h"

#include "common/manager/global_data.h"
#include "common/manager/gm_list_abi.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateGMData = 330,
};

// IDA_EA: GlobalData::gmList, allocated during GlobalData::Init.

// IDA_CALLEE_EA: 0x829996C
// IDA_CALLEE_NAME: _ZN7Gm_List16LoadGmListFromDbEv
}  // namespace

// IDA_EA: 0x844EDE6
// IDA_NAME: _ZN15DB_UpdateGMDataC2Ev
// DEMANGLED: DB_UpdateGMData::DB_UpdateGMData(void)
DB_UpdateGMData::DB_UpdateGMData() {}

// IDA_EA: 0x845CD36
// IDA_NAME: _ZN15DB_UpdateGMDataD2Ev
// DEMANGLED: DB_UpdateGMData::~DB_UpdateGMData()
// IDA_EA: 0x845CD66
// IDA_NAME: _ZN15DB_UpdateGMDataD0Ev
// DEMANGLED: DB_UpdateGMData::~DB_UpdateGMData() deleting destructor
DB_UpdateGMData::~DB_UpdateGMData() {}

// IDA_EA: 0x843982C
// IDA_NAME: _ZN15DB_UpdateGMData11makeRequestEb
// DEMANGLED: DB_UpdateGMData::makeRequest(bool)
void DB_UpdateGMData::makeRequest(bool enabled) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34657);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateGMData;
    *guard << -1;
    *guard << static_cast<unsigned char>(enabled);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843990A
// IDA_NAME: _ZN15DB_UpdateGMData8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateGMData::dispatch(int,int,Stream *)
bool DB_UpdateGMData::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned char enabled = 0;
    *stream >> enabled;
    (void)enabled;

    GlobalData::gmList->LoadGmListFromDb();
    return true;
}
