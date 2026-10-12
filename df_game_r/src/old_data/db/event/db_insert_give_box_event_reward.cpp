#include "old_data/db/event/db_insert_give_box_event_reward.h"

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

#include <time.h>


// IDA_EA: 0x815E550
// IDA_NAME: _ZN26DBInsertGiveBoxEventRewardC2Ev
// DEMANGLED: DBInsertGiveBoxEventReward::DBInsertGiveBoxEventReward(void)
DBInsertGiveBoxEventReward::DBInsertGiveBoxEventReward()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::INSERT_GIVE_BOX_EVENT_REWARD) {
}

// IDA_EA: 0x815E574
// IDA_NAME: _ZN26DBInsertGiveBoxEventRewardD2Ev
// DEMANGLED: DBInsertGiveBoxEventReward::~DBInsertGiveBoxEventReward()
//
// IDA_EA: 0x815E5A4
// IDA_NAME: _ZN26DBInsertGiveBoxEventRewardD0Ev
// DEMANGLED: DBInsertGiveBoxEventReward::~DBInsertGiveBoxEventReward()
// The natural virtual destructor supplies both original i386 destructor
// variants; D0 performs the old deleting-destructor step after D2.
DBInsertGiveBoxEventReward::~DBInsertGiveBoxEventReward() {
}

// IDA_EA: 0x815D2C8
// IDA_NAME: _ZN26DBInsertGiveBoxEventReward11makeRequestEP5CUser
// DEMANGLED: DBInsertGiveBoxEventReward::makeRequest(CUser *)
void DBInsertGiveBoxEventReward::makeRequest(CUser* user) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        873);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(
        guard,
        TaiwanInternalPack::INSERT_GIVE_BOX_EVENT_REWARD,
        user->GetUID());

    // The original invokes the concrete member-template specialization, not
    // GetInBufferUntyped/get_binary.  Its A-side provider is restored in the
    // Taiwan Stream memberization unit at 0x815ED08.
    MSG* msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = static_cast<uint32_t>(user->get_acc_id());
        msg->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBInsertGiveBoxEventReward::makeRequest(CUser*)",
            878,
            0);
        trace("[Taiwan, Event] msg is null");
    }
}

// IDA_EA: 0x815D3EE
// IDA_NAME: _ZN26DBInsertGiveBoxEventReward8dispatchEiiP6Stream
// DEMANGLED: DBInsertGiveBoxEventReward::dispatch(int,int,Stream *)
int DBInsertGiveBoxEventReward::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;

    // Old Stream::GetOutBuffer<MSG> @0x815ED1E advances the out cursor by
    // eight bytes and returns null if the frame has no complete signal.
    MSG* msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "context null";
    }

    // IDA passes literal DB handle 9 and server group 0 here.
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    // Reproduce the source's local-time reset calculation exactly: construct
    // today's fatigue/reset hour at :00:00, then move it back one day when
    // the current hour has not yet reached that configured hour.
    time_t now = time(0);
    struct tm current_tm;
    localtime_r(&now, &current_tm);
    struct tm reset_tm = current_tm;
    reset_tm.tm_hour = G_CEnvironment()->get_fatigue_time();
    reset_tm.tm_min = 0;
    reset_tm.tm_sec = 0;
    time_t reset_time = mktime(&reset_tm);
    if (current_tm.tm_hour < reset_tm.tm_hour) {
        reset_time -= 86400;
    }

    db_handle->set_query(
        "seLect m_id from event_1306_account_reward_2nd "
        "where m_id=%s and occ_date > FROM_UNIXTIME(%d)",
        NumberToString(msg->account_id, 0),
        static_cast<int>(reset_time));
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    // Only a first claim after the configured reset is inserted and returned
    // to Inter code 16.  Existing rows intentionally generate no reply.
    if (!db_handle->get_n_rows()) {
        db_handle->set_query(
            "inSert into event_1306_account_reward_2nd "
            "(m_id , charac_no , occ_date ) values (%s , %u , now())",
            NumberToString(msg->account_id, 0),
            msg->charac_no);
        if (db_handle->exec(true) != true) {
            throw "insert_exec";
        }

        Stream* reply_stream = GlobalData::stream_pool()->Acquire(
            "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
            933);
        CStreamGuard reply(reply_stream, true);
        Taiwan::internal_stream(
            reply,
            TaiwanInternalPack::INSERT_GIVE_BOX_EVENT_REWARD,
            user_idx);

        // IDA @0x815ED08: use the real typed input member template and copy
        // the original account/character pair without a raw byte facade.
        MSG* response = reply.GetInBuffer<MSG>();
        if (!response) {
            throw "stream";
        }
        response->account_id = msg->account_id;
        response->charac_no = msg->charac_no;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, reply);
    }

    return 1;
}
