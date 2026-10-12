#include "old_data/Login/inter_returns/inter_ready_to_terminate.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x84C7D2C
// IDA_NAME: _ZN22Inter_ReadyToTerminate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReadyToTerminate::dispatch_sig(CUser *,char *,int)
int Inter_ReadyToTerminate::dispatch_sig(CUser* /*user*/,
                                         char* /*payload*/,
                                         int /*payload_size*/) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("InterDispatcher.cpp", 8277);
    CStreamGuard guard(stream, true);
    *guard << 63;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x84EA0DA
// IDA_NAME: _ZN22Inter_ReadyToTerminateC2Ev
// DEMANGLED: Inter_ReadyToTerminate::Inter_ReadyToTerminate(void)
Inter_ReadyToTerminate::Inter_ReadyToTerminate() : ISigDispatcher() {}

// IDA_EA: 0x84EA0F6 / 0x84EA126
// IDA_NAME: _ZN22Inter_ReadyToTerminateD2Ev / D0Ev
// DEMANGLED: Inter_ReadyToTerminate::~Inter_ReadyToTerminate()
Inter_ReadyToTerminate::~Inter_ReadyToTerminate() {}
