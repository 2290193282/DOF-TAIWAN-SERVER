#include "old_data/db/account/db_force_save_simple_sso.h"

#include "common/manager/global_data.h"
#include "old_data/Login/simple_sso.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844E71E
// IDA_NAME: _ZN21DB_ForceSaveSimpleSSOC2Ev
// DEMANGLED: DB_ForceSaveSimpleSSO::DB_ForceSaveSimpleSSO(void)
DB_ForceSaveSimpleSSO::DB_ForceSaveSimpleSSO() {}

// IDA_EA: 0x845E8A2
// IDA_NAME: _ZN21DB_ForceSaveSimpleSSOD2Ev
// DEMANGLED: DB_ForceSaveSimpleSSO::~DB_ForceSaveSimpleSSO()
// IDA_EA: 0x845E8D2
// IDA_NAME: _ZN21DB_ForceSaveSimpleSSOD0Ev
// DEMANGLED: DB_ForceSaveSimpleSSO::~DB_ForceSaveSimpleSSO() deleting destructor
DB_ForceSaveSimpleSSO::~DB_ForceSaveSimpleSSO() {}

// IDA_EA: 0x8428154
// IDA_NAME: _ZN21DB_ForceSaveSimpleSSO8dispatchEiiP6Stream
// DEMANGLED: DB_ForceSaveSimpleSSO::dispatch(int,int,Stream *)
bool DB_ForceSaveSimpleSSO::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    int human_certify = 0;
    *stream >> account_id;
    *stream >> human_certify;
    return GlobalData::s_psimpleSSO &&
           GlobalData::s_psimpleSSO->updateHumanCertify(account_id,
                                                       human_certify) == true;
}

// IDA_EA: 0x84281AE
// IDA_NAME: _ZN21DB_ForceSaveSimpleSSO11makeRequestEji
// DEMANGLED: DB_ForceSaveSimpleSSO::makeRequest(unsigned int,int)
void DB_ForceSaveSimpleSSO::makeRequest(unsigned int account_id,
                                        int human_certify) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27057);
    CStreamGuard guard(stream, true);
    *guard << 207;
    *guard << -1;
    *guard << account_id;
    *guard << human_certify;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
