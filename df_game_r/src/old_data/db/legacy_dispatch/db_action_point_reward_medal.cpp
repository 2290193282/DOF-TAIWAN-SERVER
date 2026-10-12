#include "old_data/db/legacy_dispatch/db_action_point_reward_medal.h"
#include "old_data/db/core/action_point_db_exchange_proc_abi.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <vector>

namespace APSystem {

extern long getDailyInitializeTime(int hour, int minute, int second)
    __asm__("_Z22getDailyInitializeTimeiii");

namespace {

enum {
    kSigLoadRewardMedalSize = 0x13,
    kSigUpdateRewardMedalSize = 0x0F,
};

}  // namespace

// IDA_EA: 0x844DDB2
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedalC2Ev
// DEMANGLED: APSystem::DB_LoadRewardMedal::DB_LoadRewardMedal(void)
DB_LoadRewardMedal::DB_LoadRewardMedal() {}

// IDA_EA: 0x81254E8
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedalD2Ev
// DEMANGLED: APSystem::DB_LoadRewardMedal::~DB_LoadRewardMedal()
// IDA_EA: 0x8125518
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedalD0Ev
// DEMANGLED: APSystem::DB_LoadRewardMedal::~DB_LoadRewardMedal() deleting destructor
DB_LoadRewardMedal::~DB_LoadRewardMedal() {}

// IDA_EA: 0x812487A
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedal11makeRequestEij
// DEMANGLED: APSystem::DB_LoadRewardMedal::makeRequest(int,unsigned int)
void DB_LoadRewardMedal::makeRequest(int user_idx, unsigned int charac_no) {
    (void)user_idx;
    (void)charac_no;
}

// IDA_EA: 0x8124882
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedal8dispatchEiiP6Stream
// DEMANGLED: APSystem::DB_LoadRewardMedal::dispatch(int,int,Stream *)
bool DB_LoadRewardMedal::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    if (!stream->enable_out(kSigLoadRewardMedalSize)) {
        return false;
    }
    _SIG_LOAD_REWARD_MEDAL* payload =
        static_cast<_SIG_LOAD_REWARD_MEDAL*>(
            static_cast<void*>(stream->get_buffer()));
    stream->out_ptr(kSigLoadRewardMedalSize);
    if (Select(*payload)) {
        return SendResult(signal, user_idx, *payload);
    }
    return false;
}

// IDA_EA: 0x8124910
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedal6SelectERNS_22_SIG_LOAD_REWARD_MEDALE
// DEMANGLED: APSystem::DB_LoadRewardMedal::Select(APSystem::_SIG_LOAD_REWARD_MEDAL &)
bool DB_LoadRewardMedal::Select(_SIG_LOAD_REWARD_MEDAL& payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    std::time_t timer =
        static_cast<std::time_t>(getDailyInitializeTime(reset_hour, 0, 0));
    std::time_t prev_timer = timer - 604800;
    struct tm daily_time;
    struct tm prev_time;
    localtime_r(&timer, &daily_time);
    localtime_r(&prev_timer, &prev_time);

    char date[16];
    char prev_date[16];
    std::sprintf(date,
                 "%04d-%02d-%02d",
                 daily_time.tm_year + 1900,
                 daily_time.tm_mon + 1,
                 daily_time.tm_mday);
    std::sprintf(prev_date,
                 "%04d-%02d-%02d",
                 prev_time.tm_year + 1900,
                 prev_time.tm_mon + 1,
                 prev_time.tm_mday);

    CDataManager* data_manager = G_CDataManager();
    CActionPointEtcParameter* parameter =
        data_manager ? data_manager->GetActionPointEtcParameter(
                           CActionPointManager::_IsGMOn)
                     : 0;
    if (!parameter) {
        return false;
    }

    const std::vector<_MedalRewardItem>& rewards =
        parameter->medal_rewards();
    int min_ap_sum = 99999;
    for (std::vector<_MedalRewardItem>::const_iterator it =
             rewards.begin();
         it != rewards.end();
         ++it) {
        if (it->medal_type < min_ap_sum) {
            min_ap_sum = it->medal_type;
        }
    }

    db_handle->set_query(
        "select occ_date, ap_sum  \t\t\t\t\tfrom charac_action_point\t\t\t\t\t\twhere charac_no = %u\t\t\t\t\t\tand is_reward_medal = %u \t\t\t\t\tand occ_date < '%s'\t\t\t\t\t\tand occ_date >= '%s'\t\t\t\t\t\tand ap_sum >= %d",
        payload.charac_no,
        0,
        date,
        prev_date,
        min_ap_sum);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() <= 0) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_str(0, payload.occ_date, sizeof(payload.occ_date)) !=
        true) {
        return false;
    }
    return db_handle->get_uint(1, payload.ap_sum) == true;
}

// IDA_EA: 0x8124B8E
// IDA_NAME: _ZN8APSystem18DB_LoadRewardMedal10SendResultEiiRNS_22_SIG_LOAD_REWARD_MEDALE
// DEMANGLED: APSystem::DB_LoadRewardMedal::SendResult(int,int,APSystem::_SIG_LOAD_REWARD_MEDAL &)
bool DB_LoadRewardMedal::SendResult(int signal,
                                    int user_idx,
                                    _SIG_LOAD_REWARD_MEDAL& payload) {
    Stream* out_stream = GlobalData::stream_pool()->Acquire(
        "localchina/localchina_ActionPointSystem_Dispatcher.cpp", 448);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(&payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844DDCE
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedalC2Ev
// DEMANGLED: APSystem::DB_UpdateRewardMedal::DB_UpdateRewardMedal(void)
DB_UpdateRewardMedal::DB_UpdateRewardMedal() {}

// IDA_EA: 0x8125536
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedalD2Ev
// DEMANGLED: APSystem::DB_UpdateRewardMedal::~DB_UpdateRewardMedal()
// IDA_EA: 0x8125566
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedalD0Ev
// DEMANGLED: APSystem::DB_UpdateRewardMedal::~DB_UpdateRewardMedal() deleting destructor
DB_UpdateRewardMedal::~DB_UpdateRewardMedal() {}

// IDA_EA: 0x8124C70
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedal11makeRequestEiRKNS_22_SIG_LOAD_REWARD_MEDALE
// DEMANGLED: APSystem::DB_UpdateRewardMedal::makeRequest(int,APSystem::_SIG_LOAD_REWARD_MEDAL const&)
void DB_UpdateRewardMedal::makeRequest(
    int user_idx,
    const _SIG_LOAD_REWARD_MEDAL& payload) {
    (void)user_idx;
    (void)payload;
}

// IDA_EA: 0x8124C78
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedal8dispatchEiiP6Stream
// DEMANGLED: APSystem::DB_UpdateRewardMedal::dispatch(int,int,Stream *)
bool DB_UpdateRewardMedal::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream->enable_out(kSigUpdateRewardMedalSize)) {
        return false;
    }
    _SIG_UPDATE_REWARD_MEDAL* payload =
        static_cast<_SIG_UPDATE_REWARD_MEDAL*>(
            static_cast<void*>(stream->get_buffer()));
    stream->out_ptr(kSigUpdateRewardMedalSize);
    Update(*payload);
    return true;
}

// IDA_EA: 0x8124CB2
// IDA_NAME: _ZN8APSystem20DB_UpdateRewardMedal6UpdateERNS_24_SIG_UPDATE_REWARD_MEDALE
// DEMANGLED: APSystem::DB_UpdateRewardMedal::Update(APSystem::_SIG_UPDATE_REWARD_MEDAL &)
bool DB_UpdateRewardMedal::Update(_SIG_UPDATE_REWARD_MEDAL& payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "update charac_action_point set is_reward_medal=%u \t\t\t\t\twhere charac_no = %u\t\t\t\t\t\tand is_reward_medal = %u \t\t\t\t\tand occ_date = '%s'",
        1,
        payload.charac_no,
        0,
        payload.occ_date);
    return db_handle->exec(true) == true;
}

}  // namespace APSystem
