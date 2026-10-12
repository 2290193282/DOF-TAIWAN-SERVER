#include "old_data/db/legacy_dispatch/db_action_point_update.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "infra/network/packet/stream.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>

namespace APSystem {

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len)
    __asm__("_Z12compress_zipPcRjPKcj");
extern tm* __GetCurrentDNFDate(tm& out_time)
    __asm__("_Z19__GetCurrentDNFDateR2tm");

namespace {

enum {
    kActionPointClearZipSize = 0x800,
};

char g_szZipAP_CLEAR_STATE[kActionPointClearZipSize];

}  // namespace

// IDA_EA: 0x844DD96
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPointC2Ev
// DEMANGLED: APSystem::DB_UpdateActionPoint::DB_UpdateActionPoint(void)
DB_UpdateActionPoint::DB_UpdateActionPoint() {}

// IDA_EA: 0x812549A
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPointD2Ev
// DEMANGLED: APSystem::DB_UpdateActionPoint::~DB_UpdateActionPoint()
// IDA_EA: 0x81254CA
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPointD0Ev
// DEMANGLED: APSystem::DB_UpdateActionPoint::~DB_UpdateActionPoint() deleting destructor
DB_UpdateActionPoint::~DB_UpdateActionPoint() {}

// IDA_EA: 0x812431C
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPoint11makeRequestEiRKNS_22_SIG_LOAD_ACTION_POINTEb
// DEMANGLED: APSystem::DB_UpdateActionPoint::makeRequest(int,APSystem::_SIG_LOAD_ACTION_POINT const&,bool)
const _SIG_LOAD_ACTION_POINT* DB_UpdateActionPoint::makeRequest(
    int user_idx,
    const _SIG_LOAD_ACTION_POINT& payload,
    bool previous_day) {
    (void)user_idx;
    (void)previous_day;
    return &payload;
}

// IDA_EA: 0x812432A
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPoint8dispatchEiiP6Stream
// DEMANGLED: APSystem::DB_UpdateActionPoint::dispatch(int,int,Stream *)
bool DB_UpdateActionPoint::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    bool previous_day = false;
    stream->get_binary(&previous_day, sizeof(previous_day));
    if (!stream->enable_out(sizeof(_SIG_LOAD_ACTION_POINT))) {
        return false;
    }
    _SIG_LOAD_ACTION_POINT* payload =
        reinterpret_cast<_SIG_LOAD_ACTION_POINT*>(stream->get_buffer());
    stream->out_ptr(sizeof(*payload));
    Update(previous_day, *payload);
    return true;
}

// IDA_EA: 0x8124386
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPoint6UpdateEbRKNS_22_SIG_LOAD_ACTION_POINTE
// DEMANGLED: APSystem::DB_UpdateActionPoint::Update(bool,APSystem::_SIG_LOAD_ACTION_POINT const&)
bool DB_UpdateActionPoint::Update(bool previous_day,
                                  const _SIG_LOAD_ACTION_POINT& payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));

    struct tm dnf_time;
    if (previous_day) {
        std::time_t timer = 0;
        CSystemTime* system_time = GlobalData::system_time();
        if (system_time) {
            timer = static_cast<std::time_t>(system_time->getCurSec()) - 86400;
        } else {
            std::time(&timer);
            timer -= 86400;
        }
        localtime_r(&timer, &dnf_time);
    } else {
        __GetCurrentDNFDate(dnf_time);
    }

    char date[12];
    std::memset(date, 0, sizeof(date));
    std::sprintf(date,
                 "%04d-%02d-%02d",
                 dnf_time.tm_year + 1900,
                 dnf_time.tm_mon + 1,
                 dnf_time.tm_mday);

    std::string clear_state;
    MakeActionClearStateString(
        clear_state,
        payload.action_points,
        _SIG_LOAD_ACTION_POINT::kActionPointCount);

    unsigned int zip_length = 1024;
    std::memset(g_szZipAP_CLEAR_STATE, 0, sizeof(g_szZipAP_CLEAR_STATE));
    if (compress_zip(g_szZipAP_CLEAR_STATE,
                     zip_length,
                     clear_state.c_str(),
                     static_cast<unsigned int>(clear_state.size())) != 1) {
        return false;
    }

    char* clear_blob = db_handle->blob_to_str(
        0, g_szZipAP_CLEAR_STATE, static_cast<int>(zip_length));
    db_handle->set_query(
        "update charac_action_point set ap_sum=%d, is_reward_medal=%u, \t\t\t\t\tis_reward_item_1=%u, is_reward_item_2=%u, is_reward_item_3=%u, is_reward_item_4=%u,\t\t\t\t\t\tap_clear_state='%s'\t\t\t\t\t\twhere occ_date='%s'\t\t\t\t\t\tand charac_no=%u",
        payload.ap_sum,
        payload.reward_medal,
        payload.reward_item_1,
        payload.reward_item_2,
        payload.reward_item_3,
        payload.reward_item_4,
        clear_blob,
        date,
        payload.charac_no);
    bool exec_result = db_handle->exec(true);
    if (exec_result != true || !db_handle->getAffectedRowCount()) {
        clear_blob = db_handle->blob_to_str(
            0, g_szZipAP_CLEAR_STATE, static_cast<int>(zip_length));
        db_handle->set_query(
            "insert into charac_action_point\t\t\t\t\t\t\t(occ_date, charac_no, ap_sum, is_reward_medal, \t\t\t\t\t\t\tis_reward_item_1, is_reward_item_2, is_reward_item_3, is_reward_item_4,\t\t\t\t\t\t\t\tap_clear_state)\t\t\t\t\t\t\t\tvalues \t\t\t\t\t\t\t('%s', %u, %u, %u, %u, %u, %u, %u, '%s')",
            date,
            payload.charac_no,
            payload.ap_sum,
            payload.reward_medal,
            payload.reward_item_1,
            payload.reward_item_2,
            payload.reward_item_3,
            payload.reward_item_4,
            clear_blob);
        exec_result = db_handle->exec(true);
    }
    return exec_result == true;
}

// IDA_EA: 0x81246D6
// IDA_NAME: _ZN8APSystem20DB_UpdateActionPoint26MakeActionClearStateStringERSsPKNS_12_ActionPointEj
// DEMANGLED: APSystem::DB_UpdateActionPoint::MakeActionClearStateString(std::string &,APSystem::_ActionPoint const*,unsigned int)
void DB_UpdateActionPoint::MakeActionClearStateString(
    std::string& text,
    const _ActionPoint* action_points,
    unsigned int count) {
    text.clear();
    std::string unused;
    char line[128];
    for (unsigned int index = 0; index < count; ++index) {
        if (CActionPointEtcParameter::IsValidActionGroupIndex(
                static_cast<_ActionGroupIndex>(
                    action_points[index].action_group_index)) == true) {
            std::sprintf(line,
                         "%d,%d,%d,%d,%d,%d,%d|",
                         action_points[index].action_index,
                         action_points[index].action_group_index,
                         action_points[index].clear_state[0],
                         action_points[index].clear_state[1],
                         action_points[index].clear_state[2],
                         action_points[index].clear_state[3],
                         action_points[index].clear_state[4]);
            text += line;
        }
    }
}

}  // namespace APSystem
