#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x815E46C
// IDA_NAME: _ZN25DBSelectPcroomDailyRewardC2Ev
// DEMANGLED: DBSelectPcroomDailyReward::DBSelectPcroomDailyReward(void)
DBSelectPcroomDailyReward::DBSelectPcroomDailyReward()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SELECT_PCROOM_DAILY_REWARD) {
}

// IDA_EA: 0x815E490 (D2), 0x815E4C0 (D0)
// IDA_NAME: _ZN25DBSelectPcroomDailyRewardD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBSelectPcroomDailyReward::~DBSelectPcroomDailyReward() {
}

// IDA_EA: 0x815E4DE
// IDA_NAME: _ZN25DBUpdatePcroomDailyRewardC2Ev
// DEMANGLED: DBUpdatePcroomDailyReward::DBUpdatePcroomDailyReward(void)
DBUpdatePcroomDailyReward::DBUpdatePcroomDailyReward()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::UPDATE_PCROOM_DAILY_REWARD) {
}

// IDA_EA: 0x815E502 (D2), 0x815E532 (D0)
// IDA_NAME: _ZN25DBUpdatePcroomDailyRewardD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBUpdatePcroomDailyReward::~DBUpdatePcroomDailyReward() {
}

// IDA_EA: 0x815C986
// IDA_NAME: _ZN25DBSelectPcroomDailyReward11makeRequestEP5CUser
// DEMANGLED: DBSelectPcroomDailyReward::makeRequest(CUser *)
void DBSelectPcroomDailyReward::makeRequest(CUser* user) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        729);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 13, user->GetUID());
    MSG* msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = user->get_acc_id();
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBSelectPcroomDailyReward::makeRequest(CUser*)",
            735,
            0);
        trace("[Taiwan, Pcroom] msg is null");
    }
}

// IDA_EA: 0x815CA94
// IDA_NAME: _ZN25DBSelectPcroomDailyReward8dispatchEiiP6Stream
// DEMANGLED: DBSelectPcroomDailyReward::dispatch(int,int,Stream *)
int DBSelectPcroomDailyReward::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;

    MSG* msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "msg is null";
    }

    unsigned int selected_m_id = 0;
    unsigned int occ_time = 0;
    char remain_reward_count = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    const char* account_id_text = NumberToString(msg->account_id, 0);
    db_handle->set_query(
        "seLect m_id, remain_reward_count, unix_timestamp(occ_date) FROM pcroom_daily_reward_tw WHERE m_id=%s",
        account_id_text);
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            throw "fetch";
        }
        if (db_handle->get_int(0, selected_m_id) != true) {
            throw "get_int 0";
        }
        if (db_handle->get_byte(1, remain_reward_count) != true) {
            throw "get_int 1";
        }
        if (db_handle->get_int(2, occ_time) != true) {
            throw "get_int 2";
        }

        const int cur_sec = GlobalData::system_time()->getCurSec();
        const int fatigue_time = G_CEnvironment()->get_fatigue_time();
        if (CheckDailyScheduleTime(fatigue_time, occ_time, cur_sec)) {
            InterSelectPcroomDailyReward::makeRequest(user_idx,
                                                      selected_m_id,
                                                      4);
            account_id_text = NumberToString(msg->account_id, 0);
            cMyTrace trace(
                "virtual bool DBSelectPcroomDailyReward::dispatch(int, int, Stream*)",
                785,
                0);
            trace("[Taiwan, Pcroom] pcroom_daily_reward_tw yesterday m_id:%s",
                  account_id_text);
        } else {
            InterSelectPcroomDailyReward::makeRequest(user_idx,
                                                      selected_m_id,
                                                      remain_reward_count);
            account_id_text = NumberToString(msg->account_id, 0);
            cMyTrace trace(
                "virtual bool DBSelectPcroomDailyReward::dispatch(int, int, Stream*)",
                790,
                0);
            trace("[Taiwan, Pcroom] pcroom_daily_reward_tw today m_id:%s, remain_reward_count:%d",
                  account_id_text,
                  remain_reward_count);
        }
    } else {
        InterSelectPcroomDailyReward::makeRequest(user_idx,
                                                  selected_m_id,
                                                  4);
        account_id_text = NumberToString(msg->account_id, 0);
        cMyTrace trace(
            "virtual bool DBSelectPcroomDailyReward::dispatch(int, int, Stream*)",
            768,
            0);
        trace("[Taiwan, Pcroom] pcroom_daily_reward_tw get_n_row==0 m_id:%s",
              account_id_text);
    }

    return 1;
}

// IDA_EA: 0x815CF06
// IDA_NAME: _ZN25DBUpdatePcroomDailyReward11makeRequestEP5CUser
// DEMANGLED: DBUpdatePcroomDailyReward::makeRequest(CUser *)
void DBUpdatePcroomDailyReward::makeRequest(CUser* user) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        805);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 14, user->GetUID());
    MSG* msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = user->get_acc_id();
        msg->remain_reward_count = user->getPcroomDailyRewardRemain();
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBUpdatePcroomDailyReward::makeRequest(CUser*)",
            811,
            0);
        trace("[Taiwan, Pcroom] msg is null");
    }
}

// IDA_EA: 0x815D024
// IDA_NAME: _ZN25DBUpdatePcroomDailyReward8dispatchEiiP6Stream
// DEMANGLED: DBUpdatePcroomDailyReward::dispatch(int,int,Stream *)
int DBUpdatePcroomDailyReward::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MSG* msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "context null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    const char* account_id_text = NumberToString(msg->account_id, 0);
    db_handle->set_query(
        "seLect m_id from pcroom_daily_reward_tw where m_id=%s",
        account_id_text);
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    if (db_handle->get_n_rows()) {
        account_id_text = NumberToString(msg->account_id, 0);
        db_handle->set_query(
            "upDate pcroom_daily_reward_tw set occ_date=now(), remain_reward_count=%d where m_id = %s",
            msg->remain_reward_count,
            account_id_text);
        if (db_handle->exec(true) != true) {
            throw "update_exec";
        }
    } else {
        account_id_text = NumberToString(msg->account_id, 0);
        db_handle->set_query(
            "inSert into pcroom_daily_reward_tw(m_id, occ_date, remain_reward_count) values('%s', now(), %d)",
            account_id_text,
            msg->remain_reward_count);
        if (db_handle->exec(true) != true) {
            throw "insert_exec";
        }
    }

    return 1;
}
