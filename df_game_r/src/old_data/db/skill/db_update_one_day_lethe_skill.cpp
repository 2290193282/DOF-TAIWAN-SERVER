#include "old_data/db/skill/db_update_one_day_lethe_skill.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/types/compressed_blob.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_common_public_abi.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kPremiumLetheNotUseState = 255,
    kPremiumLetheUseState = 1,
    kPremiumLetheSkillConfirmState = 2,
    kPremiumLetheInitSkillState = 4,
};

}  // namespace

// IDA_EA: 0x844EE8E
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkillC2Ev
// DEMANGLED: DB_UpdateOneDayLetheSkill::DB_UpdateOneDayLetheSkill(void)
DB_UpdateOneDayLetheSkill::DB_UpdateOneDayLetheSkill() {}

// IDA_EA: 0x845D6A8
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkillD2Ev
// DEMANGLED: DB_UpdateOneDayLetheSkill::~DB_UpdateOneDayLetheSkill()
// IDA_EA: 0x845D6D8
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkillD0Ev
// DEMANGLED: DB_UpdateOneDayLetheSkill::~DB_UpdateOneDayLetheSkill() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateOneDayLetheSkill::~DB_UpdateOneDayLetheSkill() {}

// IDA_EA: 0x8433ECA
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkill8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateOneDayLetheSkill::dispatch(int,int,Stream *)
bool DB_UpdateOneDayLetheSkill::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload =
        stream->GetOutBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>();
    if (!payload) {
        return false;
    }

    bool result = true;
    switch (payload->state) {
        case kPremiumLetheNotUseState:
            result = ProcessInitLethe(payload);
            break;
        case kPremiumLetheUseState:
            result = ProcessUseLethe(payload);
            break;
        case kPremiumLetheSkillConfirmState:
            result = ProcessSkillConfirm(payload, user_idx);
            break;
        case kPremiumLetheInitSkillState:
            result = ProcessInitLetheSkill(payload);
            payload->state = kPremiumLetheInitSkillState;
            payload->lethe_flag = kPremiumLetheUseState;
            break;
        default:
            return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 31916);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_PREMIUM_LETHE_FLAG* out_payload =
        guard.GetInBuffer<SIG_PREMIUM_LETHE_FLAG>();
    if (!out_payload) {
        return false;
    }
    out_payload->success = result ? 1u : 0u;
    out_payload->state = payload->state;
    memcpy(out_payload->skill_slot,
           payload->skill_slot,
           sizeof(out_payload->skill_slot));
    out_payload->lethe_flag = payload->lethe_flag;
    out_payload->skill_tree_kind = payload->skill_tree_kind;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843410C
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkill16ProcessInitLetheEP35SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD
// DEMANGLED: DB_UpdateOneDayLetheSkill::ProcessInitLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD *)
bool DB_UpdateOneDayLetheSkill::ProcessInitLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload_data->skill_tree_kind != kPremiumLetheNotUseState &&
        payload_data->skill_tree_kind != 0) {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query(
            "upDate %s set lethe_flag_2nd=%u where charac_no=%u",
            skill_table,
            payload_data->lethe_flag,
            payload_data->charac_no);
    } else {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query("upDate %s set lethe_flag=%u where charac_no=%u",
                             skill_table,
                             payload_data->lethe_flag,
                             payload_data->charac_no);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84341FC
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkill15ProcessUseLetheEP35SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD
// DEMANGLED: DB_UpdateOneDayLetheSkill::ProcessUseLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD *)
bool DB_UpdateOneDayLetheSkill::ProcessUseLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* slot_blob = 0;
    if (put_compressed_blob_data(db_handle,
                                 0,
                                 payload_data->skill_slot,
                                 sizeof(payload_data->skill_slot),
                                 &slot_blob) != 1) {
        return false;
    }
    if (payload_data->skill_tree_kind != kPremiumLetheNotUseState &&
        payload_data->skill_tree_kind != 0) {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query(
            "upDate %s set skill_slot_lethe_2nd='%s', lethe_flag_2nd=%u where charac_no=%u",
            skill_table,
            slot_blob,
            payload_data->lethe_flag,
            payload_data->charac_no);
    } else {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query(
            "upDate %s set skill_slot_lethe='%s', lethe_flag=%u where charac_no=%u",
            skill_table,
            slot_blob,
            payload_data->lethe_flag,
            payload_data->charac_no);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8434340
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkill19ProcessSkillConfirmEP35SIG_RESULT_PREMIUM_LETHE_SKILL_LOADi
// DEMANGLED: DB_UpdateOneDayLetheSkill::ProcessSkillConfirm(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD *,int)
bool DB_UpdateOneDayLetheSkill::ProcessSkillConfirm(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload_data,
                                                    int user_idx) {
    (void)user_idx;

    MySQL* premium_db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    MySQL* skill_db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* slot_blob = 0;
    if (put_compressed_blob_data(skill_db_handle,
                                 0,
                                 payload_data->skill_slot,
                                 sizeof(payload_data->skill_slot),
                                 &slot_blob) != 1) {
        return false;
    }
    if (payload_data->skill_tree_kind != kPremiumLetheNotUseState &&
        payload_data->skill_tree_kind) {
        const char* skill_table = get_skill_table_name(0);
        skill_db_handle->set_query(
            "upDate %s set skill_slot_lethe_2nd='%s', lethe_flag_2nd=%u  where charac_no=%u",
            skill_table,
            slot_blob,
            kPremiumLetheSkillConfirmState,
            payload_data->charac_no);
    } else {
        const char* skill_table = get_skill_table_name(0);
        skill_db_handle->set_query(
            "upDate %s set skill_slot_lethe='%s', lethe_flag=%u  where charac_no=%u",
            skill_table,
            slot_blob,
            kPremiumLetheSkillConfirmState,
            payload_data->charac_no);
    }
    if (skill_db_handle->exec(true) != true) {
        return false;
    }

    cMyTrace trace_flag(
        "bool DB_UpdateOneDayLetheSkill::ProcessSkillConfirm(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD*, int)",
        32026,
        0);
    trace_flag("ONE_DAY_LETHE : DB_CONFIRM_SKILL-FLAG_WRITE");

    CDataManager* data_manager = G_CDataManager();
    CItemList* item_list = data_manager ? data_manager->item_list_ : 0;
    int restrict_code = item_list ? item_list->GetRestrictCode(197) : 0;
    const unsigned int premium_type = 33;
    const char* account_id = NumberToString(payload_data->account_id, 0);
    // ProcessSkillConfirm @ 0x8434340 reads CEnvironment+888 for the
    // member_premium.server_id predicate.
    const int server_group = G_CEnvironment()->get_server_group();
    premium_db_handle->set_query(
        "upDate member_premium set service_end=now() where event_id=%u and pre_type=%u and m_id=%s and server_id in(0,%u) and service_end>now()",
        restrict_code,
        premium_type,
        account_id,
        server_group);
    if (premium_db_handle->exec(true) != true) {
        return false;
    }

    cMyTrace trace_premium(
        "bool DB_UpdateOneDayLetheSkill::ProcessSkillConfirm(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD*, int)",
        32041,
        0);
    trace_premium("ONE_DAY_LETHE : DB_CONFIRM_SKILL-PREMIUM_END_CHANGE");
    return true;
}

// IDA_EA: 0x84345A4
// IDA_NAME: _ZN25DB_UpdateOneDayLetheSkill21ProcessInitLetheSkillEP35SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD
// DEMANGLED: DB_UpdateOneDayLetheSkill::ProcessInitLetheSkill(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD *)
bool DB_UpdateOneDayLetheSkill::ProcessInitLetheSkill(
    SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* slot_blob = 0;
    if (put_compressed_blob_data(db_handle,
                                 0,
                                 payload_data->skill_slot,
                                 sizeof(payload_data->skill_slot),
                                 &slot_blob) != 1) {
        return false;
    }
    if (payload_data->skill_tree_kind != kPremiumLetheNotUseState &&
        payload_data->skill_tree_kind != 0) {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query(
            "upDate %s set skill_slot_lethe_2nd='%s', lethe_flag_2nd=%u where charac_no=%u",
            skill_table,
            slot_blob,
            kPremiumLetheUseState,
            payload_data->charac_no);
    } else {
        const char* skill_table = get_skill_table_name(static_cast<char>(0));
        db_handle->set_query(
            "upDate %s set skill_slot_lethe='%s', lethe_flag=%u where charac_no=%u",
            skill_table,
            slot_blob,
            kPremiumLetheUseState,
            payload_data->charac_no);
    }
    if (db_handle->exec(true) != true) {
        return false;
    }
    cMyTrace trace(
        "bool DB_UpdateOneDayLetheSkill::ProcessInitLetheSkill(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD*)",
        32073,
        0);
    trace("ONE_DAY_LETHE : DB_INIT_LETHE_SKILL");
    return true;
}
