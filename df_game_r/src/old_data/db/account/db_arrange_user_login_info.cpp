#include "old_data/db/account/db_arrange_user_login_info.h"

#include "common/manager/global_data.h"
#include "old_data/Login/simple_sso.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>


#pragma pack(push, 1)
struct SIG_ARRANGE_USER_LOGIN_INFO {
    uint32_t account_id;         // +0
    uint8_t expire_login_time;   // +4
    uint8_t raw_05[3];           // +5
    uint32_t user_idx;           // +8
};
#pragma pack(pop)

typedef char SIG_ARRANGE_USER_LOGIN_INFO_Size_Must_Be_12[
    (sizeof(SIG_ARRANGE_USER_LOGIN_INFO) == 12) ? 1 : -1];

// IDA_EA: 0x844E8A6
// IDA_NAME: _ZN23DB_ArrangeUserLoginInfoC2Ev
// DEMANGLED: DB_ArrangeUserLoginInfo::DB_ArrangeUserLoginInfo(void)
DB_ArrangeUserLoginInfo::DB_ArrangeUserLoginInfo() {}

// IDA_EA: 0x845E4AC
// IDA_NAME: _ZN23DB_ArrangeUserLoginInfoD2Ev
// DEMANGLED: DB_ArrangeUserLoginInfo::~DB_ArrangeUserLoginInfo()
// IDA_EA: 0x845E4DC
// IDA_NAME: _ZN23DB_ArrangeUserLoginInfoD0Ev
// DEMANGLED: DB_ArrangeUserLoginInfo::~DB_ArrangeUserLoginInfo() deleting destructor
DB_ArrangeUserLoginInfo::~DB_ArrangeUserLoginInfo() {}

// IDA_EA: 0x842ACF4
// IDA_NAME: _ZN23DB_ArrangeUserLoginInfo8dispatchEiiP6Stream
// DEMANGLED: DB_ArrangeUserLoginInfo::dispatch(int,int,Stream *)
bool DB_ArrangeUserLoginInfo::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_ARRANGE_USER_LOGIN_INFO payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    if (payload.expire_login_time == 1 && GlobalData::s_psimpleSSO) {
        GlobalData::s_psimpleSSO->expireLoginTime(payload.account_id);
    }
    return true;
}

// IDA_EA: 0x842AD30
// IDA_NAME: _ZN23DB_ArrangeUserLoginInfo11makeRequestEjbj
// DEMANGLED: DB_ArrangeUserLoginInfo::makeRequest(unsigned int,bool,unsigned int)
void DB_ArrangeUserLoginInfo::makeRequest(unsigned int account_id,
                                          bool expire_login_time,
                                          unsigned int user_idx) {
    if (!account_id) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27881);
    CStreamGuard guard(stream, true);
    *guard << 238;
    *guard << -1;

    SIG_ARRANGE_USER_LOGIN_INFO* payload =
        static_cast<SIG_ARRANGE_USER_LOGIN_INFO*>(
            guard->GetInBufferUntyped(sizeof(SIG_ARRANGE_USER_LOGIN_INFO)));
    memset(payload, 0, sizeof(SIG_ARRANGE_USER_LOGIN_INFO));
    payload->account_id = account_id;
    payload->expire_login_time = expire_login_time ? 1 : 0;
    payload->user_idx = user_idx;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
