#include "old_data/db/event/db_select_mobile_auth_reward_tw.h"

#include "old_data/Login/inter_returns/inter_select_mobile_auth_reward.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x815E3FA
// IDA_NAME: _ZN26DBSelectMobileAuthRewardTwC2Ev
// DEMANGLED: DBSelectMobileAuthRewardTw::DBSelectMobileAuthRewardTw(void)
DBSelectMobileAuthRewardTw::DBSelectMobileAuthRewardTw()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SELECT_MOBILE_AUTH_REWARD) {
}

// IDA_EA: 0x815E41E
// IDA_NAME: _ZN26DBSelectMobileAuthRewardTwD2Ev
// DEMANGLED: DBSelectMobileAuthRewardTw::~DBSelectMobileAuthRewardTw()
// IDA_EA: 0x815E44E
// IDA_NAME: _ZN26DBSelectMobileAuthRewardTwD0Ev
// DEMANGLED: DBSelectMobileAuthRewardTw::~DBSelectMobileAuthRewardTw()
// The compiler emits the deleting D0 variant for this virtual destructor.
DBSelectMobileAuthRewardTw::~DBSelectMobileAuthRewardTw() {
}

// IDA_EA: 0x815C586
// IDA_NAME: _ZN26DBSelectMobileAuthRewardTw11makeRequestEP5CUser
// DEMANGLED: DBSelectMobileAuthRewardTw::makeRequest(CUser *)
void DBSelectMobileAuthRewardTw::makeRequest(CUser* user) {
    // The original suppresses a second DB request after the reply byte at
    // expanded CUser +0x08ef81 has been set, regardless of whether rewarded.
    if (user->mobile_auth_reward_state_) {
        return;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp", 660);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard,
                            TaiwanInternalPack::SELECT_MOBILE_AUTH_REWARD,
                            user->GetUID());
    MSG* const msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = user->get_acc_id();
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBSelectMobileAuthRewardTw::makeRequest(CUser*)",
            666,
            0);
        trace("[Taiwan, MobileAuth] msg is null");
    }
}

// IDA_EA: 0x815C6A8
// IDA_NAME: _ZN26DBSelectMobileAuthRewardTw8dispatchEiiP6Stream
// DEMANGLED: DBSelectMobileAuthRewardTw::dispatch(int,int,Stream *)
int DBSelectMobileAuthRewardTw::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;

    MSG* const msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "msg is null";
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    const char* account_id = NumberToString(msg->account_id, 0);
    db_handle->set_query(
        "seLect m_id from mobile_auth_reward_tw where m_id=%s", account_id);
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    bool rewarded = true;
    if (db_handle->get_n_rows()) {
        cMyTrace trace(
            "virtual bool DBSelectMobileAuthRewardTw::dispatch(int, int, Stream*)",
            707,
            0);
        trace("[Taiwan, MobileAuth] select mobile_auth_reward_tw. m_id:%s",
              NumberToString(msg->account_id, 0));
    } else {
        account_id = NumberToString(msg->account_id, 0);
        db_handle->set_query(
            "inSert into mobile_auth_reward_tw(m_id,occ_date) VALUES(%s,now())",
            account_id);
        if (db_handle->exec(true) != true) {
            throw "insert_exec";
        }
        rewarded = false;
        cMyTrace trace(
            "virtual bool DBSelectMobileAuthRewardTw::dispatch(int, int, Stream*)",
            702,
            0);
        trace("[Taiwan, MobileAuth] insert mobile_auth_reward_tw. m_id:%s",
              NumberToString(msg->account_id, 0));
    }

    InterSelectMobileAuthReward::makeRequest(user_idx,
                                              msg->account_id,
                                              rewarded);
    return 1;
}
