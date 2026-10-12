#include "old_data/db/account/db_load_break_away_prevent_data.h"
#include "old_data/db/account/db_load_break_away_reward_data.h"

#include "old_game/activity/schedule/event_messages.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/db/account/break_away_payload.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdio.h>
#include <string.h>
#include <time.h>


namespace {

enum {
    kDbSignalLoadBreakAwayPreventData = 321,
    kDbSignalLoadBreakAwayRewardData = 322,
    kDispatchSignalBreakAwayRewardData = 323,
    kBreakAwayDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844ED22
// IDA_NAME: _ZN27DB_LoadBreakAwayPreventDataC2Ev
// DEMANGLED: DB_LoadBreakAwayPreventData::DB_LoadBreakAwayPreventData(void)
DB_LoadBreakAwayPreventData::DB_LoadBreakAwayPreventData() {}

// IDA_EA: 0x845CDD2
// IDA_NAME: _ZN27DB_LoadBreakAwayPreventDataD2Ev
// DEMANGLED: DB_LoadBreakAwayPreventData::~DB_LoadBreakAwayPreventData()
// IDA_EA: 0x845CE02
// IDA_NAME: _ZN27DB_LoadBreakAwayPreventDataD0Ev
// DEMANGLED: DB_LoadBreakAwayPreventData::~DB_LoadBreakAwayPreventData() deleting destructor
DB_LoadBreakAwayPreventData::~DB_LoadBreakAwayPreventData() {}

// IDA_EA: 0x8438920
// IDA_NAME: _ZN27DB_LoadBreakAwayPreventData11makeRequestEb
// DEMANGLED: DB_LoadBreakAwayPreventData::makeRequest(bool)
void DB_LoadBreakAwayPreventData::makeRequest(bool enabled) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34370);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLoadBreakAwayPreventData;
    *guard << -1;
    *guard << static_cast<unsigned char>(enabled);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84389FE
// IDA_NAME: _ZN27DB_LoadBreakAwayPreventData8dispatchEiiP6Stream
// DEMANGLED: DB_LoadBreakAwayPreventData::dispatch(int,int,Stream *)
bool DB_LoadBreakAwayPreventData::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned char enabled = 0;
    *stream >> enabled;

    DBBreakAwayPreventSystemPayload result;
    result.weekday_var_a = 0;
    result.weekday_var_b = 0;
    result.weekday_var_c = 0;
    result.weekend_var_x = 0;
    result.weekend_var_y = 0;
    result.weekend_var_z = 0;
    result.next_reward_day = 0;
    result.state_flag = 0;
    result.reward_count = 0;
    int manager_no = 0;
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kBreakAwayDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    char state_flag = 0;
    db_handle->set_query(
        "seLect no, weekday_var_a, weekday_var_b, weekday_var_c, weekend_var_x, weekend_var_y, weekend_var_z, next_reward_day, state_flag from churn_system_manager order by no desc limit 1");
    if (db_handle->exec(true) != true ||
        db_handle->get_n_rows() != 1 ||
        db_handle->fetch() != true ||
        db_handle->get_int(0, manager_no) != true ||
        db_handle->get_short(1, result.weekday_var_a) != true ||
        db_handle->get_short(2, result.weekday_var_b) != true ||
        db_handle->get_short(3, result.weekday_var_c) != true ||
        db_handle->get_short(4, result.weekend_var_x) != true ||
        db_handle->get_short(5, result.weekend_var_y) != true ||
        db_handle->get_short(6, result.weekend_var_z) != true ||
        db_handle->get_short(7, result.next_reward_day) != true ||
        db_handle->get_byte(8, state_flag) != true) {
        return false;
    }
    result.state_flag = static_cast<unsigned char>(state_flag);

    if (enabled == 1) {
        db_handle->set_query(
            "upDate churn_system_manager set state_flag=%d where no = %d",
            1,
            manager_no);
        if (db_handle->exec(true) != true ||
            !db_handle->getAffectedRowCount()) {
            cMyTrace trace(
                "virtual bool DB_LoadBreakAwayPreventData::dispatch(int, int, Stream*)",
                34442,
                0);
            trace("BREAK_AWAY : STATE_FALG ALREADY ON!!");
        }
        result.state_flag = enabled;

        db_handle->set_query(
            "seLect min_day, max_day, min_val, max_val, item_id, add_info, luck_point, quest_id from churn_reward_manager");
        if (db_handle->exec(true) != true) {
            return false;
        }
        const int row_count = db_handle->get_n_rows();
        for (int row = 0; row < row_count; ++row) {
            DBBreakAwayPreventRewardRow& reward = result.rewards[row];
            char min_day = 0;
            char max_day = 0;
            char quest_id = 0;
            if (db_handle->fetch() != true ||
                db_handle->get_byte(0, min_day) != true ||
                db_handle->get_byte(1, max_day) != true ||
                db_handle->get_int(2, reward.min_value) != true ||
                db_handle->get_int(3, reward.max_value) != true ||
                db_handle->get_uint(4, reward.item_id) != true ||
                db_handle->get_int(5, reward.add_info) != true ||
                db_handle->get_int(6, reward.luck_point) != true ||
                db_handle->get_byte(7, quest_id) != true) {
                return false;
            }
            reward.quest_id = static_cast<uint8_t>(quest_id);
            reward.min_day = static_cast<uint8_t>(min_day);
            reward.max_day = static_cast<uint8_t>(max_day);
        }
        result.reward_count = static_cast<unsigned int>(row_count);

        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 34501);
        CStreamGuard guard(out_stream, true);
        *guard << kDbSignalLoadBreakAwayPreventData;
        *guard << -1;
        SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* payload =
            guard->GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>();
        memcpy(payload, &result, sizeof(result));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
        return true;
    }

    db_handle->set_query(
        "upDate churn_system_manager set state_flag=%d where no = %d",
        static_cast<int>(enabled),
        manager_no);
    if (db_handle->exec(true) != true ||
        !db_handle->getAffectedRowCount()) {
        cMyTrace trace(
            "virtual bool DB_LoadBreakAwayPreventData::dispatch(int, int, Stream*)",
            34436,
            0);
        trace("BREAK_AWAY : STATE_FALG ALREADY OFF!!");
    }
    return false;
}

// IDA_EA: 0x844ED3E
// IDA_NAME: _ZN26DB_LoadBreakAwayRewardDataC2Ev
// DEMANGLED: DB_LoadBreakAwayRewardData::DB_LoadBreakAwayRewardData(void)
DB_LoadBreakAwayRewardData::DB_LoadBreakAwayRewardData() {}

// IDA_EA: 0x845CD84
// IDA_NAME: _ZN26DB_LoadBreakAwayRewardDataD2Ev
// DEMANGLED: DB_LoadBreakAwayRewardData::~DB_LoadBreakAwayRewardData()
// IDA_EA: 0x845CDB4
// IDA_NAME: _ZN26DB_LoadBreakAwayRewardDataD0Ev
// DEMANGLED: DB_LoadBreakAwayRewardData::~DB_LoadBreakAwayRewardData() deleting destructor
DB_LoadBreakAwayRewardData::~DB_LoadBreakAwayRewardData() {}

// IDA_EA: 0x843915C
// IDA_NAME: _ZN26DB_LoadBreakAwayRewardData11makeRequestEijjR18STBreakAwayRewards
// DEMANGLED: DB_LoadBreakAwayRewardData::makeRequest(int,unsigned int,unsigned int,STBreakAwayRewards &)
void DB_LoadBreakAwayRewardData::makeRequest(
    int user_idx,
    unsigned int account_id,
    unsigned int charac_no,
    STBreakAwayRewards& rewards) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34515);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLoadBreakAwayRewardData;
    *guard << user_idx;
    *guard << account_id;
    *guard << charac_no;
    *guard << rewards.item_id;
    *guard << rewards.item_count;
    *guard << rewards.luck_point;
    *guard << rewards.reward_order;
    *guard << rewards.reward_point;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84392DC
// IDA_NAME: _ZN26DB_LoadBreakAwayRewardData8dispatchEiiP6Stream
// DEMANGLED: DB_LoadBreakAwayRewardData::dispatch(int,int,Stream *)
bool DB_LoadBreakAwayRewardData::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;

    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    unsigned int item_id = 0;
    int add_info = 0;
    int luck_point = 0;
    int reward_order = 0;
    unsigned int cera = 0;
    const time_t current_time = GlobalData::s_systemTime_.getCurSec();
    tm time_info;
    localtime_r(&current_time, &time_info);
    char history_table_suffix[7];
    sprintf(history_table_suffix,
            "%d%02d",
            time_info.tm_year + 1900,
            time_info.tm_mon + 1);

    *stream >> account_id;
    *stream >> charac_no;
    *stream >> item_id;
    *stream >> add_info;
    *stream >> luck_point;
    *stream >> reward_order;
    *stream >> cera;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kBreakAwayDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    if (reward_order) {
        switch (reward_order) {
            case 1:
                db_handle->set_query(
                    "upDate churn_member_info set last_reward_time =%u, luck_point=%d where m_id=%u",
                    current_time,
                    luck_point,
                    account_id);
                break;
            case 2:
                db_handle->set_query(
                    "upDate churn_member_info set last_reward_time =%u, second_reward_time = %u, quest_time =%u, luck_point=%d, add_info=%d where m_id=%s",
                    current_time,
                    current_time,
                    current_time + 86400,
                    luck_point,
                    3,
                    NumberToString(account_id, 0));
                break;
            case 3:
                db_handle->set_query(
                    "upDate churn_member_info set last_reward_time =%u, luck_point=%d where m_id=%s",
                    current_time,
                    luck_point,
                    NumberToString(account_id, 0));
                break;
            case 4:
                db_handle->set_query(
                    "upDate churn_member_info set last_reward_time =%u, luck_point=%d, add_info=%d where m_id=%s",
                    current_time,
                    luck_point,
                    -1,
                    NumberToString(account_id, 0));
                break;
            default:
                return false;
        }
    } else {
        db_handle->set_query(
            "upDate churn_member_info set last_reward_time =%u, first_reward_time = %u, quest_time =%u, luck_point=%d, add_info=%d where m_id=%s",
            current_time,
            current_time,
            current_time + 86400,
            luck_point,
            3,
            NumberToString(account_id, 0));
    }

    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        cMyTrace trace(
            "virtual bool DB_LoadBreakAwayRewardData::dispatch(int, int, Stream*)",
            34590,
            5);
        trace("BREAK_AWAY : ABNORMAL DB_LoadBreakAwayRewardData REWARD!!");
        return false;
    }

    // DB_LoadBreakAwayRewardData::dispatch @ 0x84392DC captures
    // CEnvironment+888 for churn_reward_history.server_id.
    const int server_id = G_CEnvironment()->get_server_group();
    db_handle->set_query(
        "inSert into churn_reward_history_%s(m_id,occ_time,server_id,charac_no,item_id,add_info,luck_point,cera,reward_order) values(%s, from_unixtime(%u), %d, %u, %u, %d, %d, %u, %d)",
        history_table_suffix,
        NumberToString(account_id, 0),
        current_time,
        server_id,
        charac_no,
        item_id,
        add_info,
        luck_point,
        cera,
        reward_order);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "virtual bool DB_LoadBreakAwayRewardData::dispatch(int, int, Stream*)",
            34624,
            5);
        trace("BREAK_AWAY : ABNORMAL DB_LoadBreakAwayRewardData LOG!!");
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 34629);
    CStreamGuard guard(out_stream, true);
    *guard << kDispatchSignalBreakAwayRewardData;
    *guard << user_idx;
    SIG_BREAK_AWAY_PREVENT_MESSAGE* payload =
        guard->GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>();
    payload->reward_order = static_cast<uint32_t>(reward_order);
    if (reward_order == 0 || reward_order == 2) {
        payload->next_reward_time =
            static_cast<uint32_t>(current_time + 86400);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
