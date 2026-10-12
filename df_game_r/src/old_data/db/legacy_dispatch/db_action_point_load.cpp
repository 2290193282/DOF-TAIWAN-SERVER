#include "old_data/db/legacy_dispatch/db_action_point_load.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>

namespace APSystem {

extern long getDailyInitializeTime(int hour, int minute, int second)
    __asm__("_Z22getDailyInitializeTimeiii");

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len)
    __asm__("_Z14uncompress_zipPcRjPKcj");

namespace {

enum {
    kActionPointClearZipSize = 0x800,
    kActionPointClearTextSize = 0x800,
};

char g_szZipAP_CLEAR_STATE[kActionPointClearZipSize];
char g_szStrAP_CLEAR_STATE[kActionPointClearTextSize];

}  // namespace

// IDA_EA: 0x848F5D8
// IDA_NAME: _ZN8APSystem22_SIG_LOAD_ACTION_POINTC2Ev
// DEMANGLED: APSystem::_SIG_LOAD_ACTION_POINT::_SIG_LOAD_ACTION_POINT(void)
_SIG_LOAD_ACTION_POINT::_SIG_LOAD_ACTION_POINT() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x844DD7A
// IDA_NAME: _ZN8APSystem18DB_LoadActionPointC2Ev
// DEMANGLED: APSystem::DB_LoadActionPoint::DB_LoadActionPoint(void)
DB_LoadActionPoint::DB_LoadActionPoint() {}

// IDA_EA: 0x812544C
// IDA_NAME: _ZN8APSystem18DB_LoadActionPointD2Ev
// DEMANGLED: APSystem::DB_LoadActionPoint::~DB_LoadActionPoint()
// IDA_EA: 0x812547C
// IDA_NAME: _ZN8APSystem18DB_LoadActionPointD0Ev
// DEMANGLED: APSystem::DB_LoadActionPoint::~DB_LoadActionPoint() deleting destructor
DB_LoadActionPoint::~DB_LoadActionPoint() {}

// IDA_EA: 0x8123B10
// IDA_NAME: _ZN8APSystem18DB_LoadActionPoint8dispatchEiiP6Stream
// DEMANGLED: APSystem::DB_LoadActionPoint::dispatch(int,int,Stream *)
bool DB_LoadActionPoint::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    if (!stream->enable_out(sizeof(_SIG_LOAD_ACTION_POINT))) {
        return false;
    }
    _SIG_LOAD_ACTION_POINT* payload =
        reinterpret_cast<_SIG_LOAD_ACTION_POINT*>(stream->get_buffer());
    stream->out_ptr(sizeof(*payload));

    const bool result = Select(*payload);
    return SendResult(signal, user_idx, *payload, result);
}

// IDA_EA: 0x8123B9E
// IDA_NAME: _ZN8APSystem18DB_LoadActionPoint6SelectERNS_22_SIG_LOAD_ACTION_POINTE
// DEMANGLED: APSystem::DB_LoadActionPoint::Select(APSystem::_SIG_LOAD_ACTION_POINT &)
bool DB_LoadActionPoint::Select(_SIG_LOAD_ACTION_POINT& payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    std::time_t timer =
        static_cast<std::time_t>(getDailyInitializeTime(reset_hour, 0, 0));
    struct tm daily_time;
    localtime_r(&timer, &daily_time);

    char date[16];
    std::sprintf(date,
                 "%04d-%02d-%02d",
                 daily_time.tm_year + 1900,
                 daily_time.tm_mon + 1,
                 daily_time.tm_mday);
    db_handle->set_query(
        "select ap_sum, is_reward_medal, is_reward_item_1, is_reward_item_2, is_reward_item_3, is_reward_item_4, ap_clear_state  from charac_action_point  where charac_no = %u  and occ_date = '%s' ",
        payload.charac_no,
        date);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() != 1) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }

    uint32_t ap_sum = 0;
    if (db_handle->get_uint(0, ap_sum) != true) {
        return false;
    }
    payload.ap_sum = ap_sum;
    unsigned char reward_flag = 0;
    if (db_handle->get_ubyte(1, reward_flag) != true) {
        return false;
    }
    payload.reward_medal = reward_flag != 0;
    if (db_handle->get_ubyte(2, reward_flag) != true) {
        return false;
    }
    payload.reward_item_1 = reward_flag != 0;
    if (db_handle->get_ubyte(3, reward_flag) != true) {
        return false;
    }
    payload.reward_item_2 = reward_flag != 0;
    if (db_handle->get_ubyte(4, reward_flag) != true) {
        return false;
    }
    payload.reward_item_3 = reward_flag != 0;
    if (db_handle->get_ubyte(5, reward_flag) != true) {
        return false;
    }
    payload.reward_item_4 = reward_flag != 0;

    const int binary_length = db_handle->get_binary_length(6);
    if (binary_length > kActionPointClearZipSize - 1) {
        return false;
    }
    std::memset(g_szZipAP_CLEAR_STATE, 0, sizeof(g_szZipAP_CLEAR_STATE));
    if (db_handle->get_binary(6,
                              g_szZipAP_CLEAR_STATE,
                              binary_length) != true) {
        return false;
    }

    unsigned int raw_length =
        *reinterpret_cast<unsigned int*>(g_szZipAP_CLEAR_STATE) & 0x3FFFFFFFu;
    if (raw_length > kActionPointClearTextSize - 1) {
        return false;
    }
    std::memset(g_szStrAP_CLEAR_STATE, 0, sizeof(g_szStrAP_CLEAR_STATE));
    if (uncompress_zip(g_szStrAP_CLEAR_STATE,
                       raw_length,
                       g_szZipAP_CLEAR_STATE,
                       static_cast<unsigned int>(binary_length)) != 1) {
        return false;
    }
    return ParseActionClearState(
        g_szStrAP_CLEAR_STATE,
        payload.action_points,
        _SIG_LOAD_ACTION_POINT::kActionPointCount);
}

// IDA_EA: 0x8123F50
// IDA_NAME: _ZN8APSystem18DB_LoadActionPoint21ParseActionClearStateEPcPNS_12_ActionPointEj
// DEMANGLED: APSystem::DB_LoadActionPoint::ParseActionClearState(char *,APSystem::_ActionPoint *,unsigned int)
bool DB_LoadActionPoint::ParseActionClearState(
    char* text,
    _ActionPoint* action_points,
    unsigned int count) {
    if (!text || !action_points) {
        return false;
    }

    char* save_ptr = 0;
    char* entry = ::strtok_r(text, "|", &save_ptr);
    for (unsigned int index = 0; entry && index < count; ++index) {
        char* entry_save_ptr = 0;
        char* token = ::strtok_r(entry, ",", &entry_save_ptr);
        char* group_token = 0;
        char* clear_token = 0;
        if (token) {
            action_points[index].action_index =
                static_cast<uint16_t>(std::atoi(token));
            group_token = ::strtok_r(0, ",", &entry_save_ptr);
        }
        if (group_token) {
            action_points[index].action_group_index =
                static_cast<_ActionGroupIndex>(std::atoi(group_token));
            if (CActionPointEtcParameter::IsValidActionGroupIndex(
                    static_cast<_ActionGroupIndex>(
                        action_points[index].action_group_index)) != true) {
                return false;
            }
            clear_token = ::strtok_r(0, ",", &entry_save_ptr);
        }
        for (int clear_index = 0; clear_token && clear_index <= 4;
             ++clear_index) {
            action_points[index].clear_state[clear_index] =
                static_cast<uint8_t>(std::atoi(clear_token));
            clear_token = ::strtok_r(0, ",", &entry_save_ptr);
        }
        entry = ::strtok_r(0, "|", &save_ptr);
    }
    return true;
}

// IDA_EA: 0x8124156
// IDA_NAME: _ZN8APSystem18DB_LoadActionPoint10SendResultEiiRNS_22_SIG_LOAD_ACTION_POINTEb
// DEMANGLED: APSystem::DB_LoadActionPoint::SendResult(int,int,APSystem::_SIG_LOAD_ACTION_POINT &,bool)
bool DB_LoadActionPoint::SendResult(int signal,
                                    int user_idx,
                                    _SIG_LOAD_ACTION_POINT& payload,
                                    bool result) {
    if (result) {
        Stream* out_stream = GlobalData::stream_pool()->Acquire(
            "localchina/localchina_ActionPointSystem_Dispatcher.cpp", 177);
        CStreamGuard guard(out_stream, true);
        *guard << signal;
        *guard << user_idx;
        guard->put_binary(&payload, sizeof(payload));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        Stream* out_stream = GlobalData::stream_pool()->Acquire(
            "localchina/localchina_ActionPointSystem_Dispatcher.cpp", 188);
        CStreamGuard guard(out_stream, true);
        *guard << 18;
        *guard << user_idx;
        guard->put_binary(&payload, sizeof(payload));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}

}  // namespace APSystem
