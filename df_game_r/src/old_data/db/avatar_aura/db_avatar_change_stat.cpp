#include "old_data/db/avatar_aura/db_avatar_change_stat.h"

#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>



namespace {

enum {
    kDbSignalAvatarChangeStat = 54,
};

}  // namespace

// IDA_EA: 0x844E0AA
// IDA_NAME: _ZN19DB_AvatarChangeStatC2Ev
// DEMANGLED: DB_AvatarChangeStat::DB_AvatarChangeStat(void)
DB_AvatarChangeStat::DB_AvatarChangeStat() {}

// IDA_EA: 0x845FBD4
// IDA_NAME: _ZN19DB_AvatarChangeStatD2Ev
// DEMANGLED: DB_AvatarChangeStat::~DB_AvatarChangeStat()
// IDA_EA: 0x845FC04
// IDA_NAME: _ZN19DB_AvatarChangeStatD0Ev
// DEMANGLED: DB_AvatarChangeStat::~DB_AvatarChangeStat() deleting destructor
DB_AvatarChangeStat::~DB_AvatarChangeStat() {}

// IDA_EA: 0x840065E
// IDA_NAME: _ZN19DB_AvatarChangeStat8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarChangeStat::dispatch(int,int,Stream *)
bool DB_AvatarChangeStat::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;

    SIG_AVATAR_CHANGE_STAT* payload =
        stream->GetOutBuffer<SIG_AVATAR_CHANGE_STAT>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "upDate user_items set stat=%d,slot=%d where ui_id=%d and charac_no=%d",
        payload->stat_or_item_id.stat,
        payload->slot,
        payload->ui_id,
        payload->charac_or_loaded.charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (payload->stat_or_item_id.stat) {
        return true;
    }

    db_handle->set_query(
        "seLect slot,ui_id,it_id,unix_timestamp(reg_date),unix_timestamp(expire_date),ability_no,ipg_agency_no,jewel_socket,hidden_option,emblem_endurance from user_items where charac_no=%u and ui_id=%d",
        payload->charac_or_loaded.charac_no,
        payload->ui_id);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }

    int slot = 0;
    int ui_id = 0;
    int item_id = 0;
    unsigned int reg_date = 0;
    unsigned int expire_date = 0;
    int ability_no = 0;
    int hidden_option = 0;
    short hidden_option_short = 0;
    short emblem_endurance = 0;
    if (db_handle->get_int(0, slot) != true) return false;
    if (db_handle->get_int(1, ui_id) != true) return false;
    if (db_handle->get_int(2, item_id) != true) return false;
    if (db_handle->get_uint(3, reg_date) != true) return false;
    if (db_handle->get_uint(4, expire_date) != true) return false;
    if (db_handle->get_int(5, ability_no) != true) return false;

    SIG_AVATAR_CHANGE_STAT result;
    memset(&result, 0, sizeof(result));

    if (db_handle->get_str(
            6, result.charac_or_loaded.loaded.ipg_agency_no,
            sizeof(result.charac_or_loaded.loaded.ipg_agency_no)) != true) {
        return false;
    }
    if (db_handle->get_binary(
            7, &result.charac_or_loaded.loaded.jewel_socket,
            sizeof(result.charac_or_loaded.loaded.jewel_socket)) != true) {
        return false;
    }
    if (db_handle->get_int(8, hidden_option) != true ||
        db_handle->get_short(8, hidden_option_short) != true ||
        db_handle->get_short(9, emblem_endurance) != true) {
        return false;
    }

    result.ui_id = static_cast<uint32_t>(ui_id);
    result.stat_or_item_id.item_id = static_cast<uint32_t>(item_id);
    result.slot = static_cast<uint8_t>(slot);
    result.charac_or_loaded.loaded.ability_no =
        static_cast<int8_t>(ability_no);
    result.charac_or_loaded.loaded.reg_date = reg_date;
    result.charac_or_loaded.loaded.expire_date = expire_date;
    result.charac_or_loaded.loaded.hidden_option =
        static_cast<uint16_t>(hidden_option);
    result.charac_or_loaded.loaded.unknown_45 = 0;
    result.charac_or_loaded.loaded.expansion_or_db_row.db_row.hidden_option_copy =
        static_cast<uint16_t>(hidden_option_short);
    result.charac_or_loaded.loaded.expansion_or_db_row.db_row.emblem_endurance =
        emblem_endurance;

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 3042);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalAvatarChangeStat;
    *guard << user_idx;
    SIG_AVATAR_CHANGE_STAT* out_payload =
        reinterpret_cast<SIG_AVATAR_CHANGE_STAT*>(guard.GetInBufferUntyped(
            sizeof(SIG_AVATAR_CHANGE_STAT)));
    if (!out_payload) {
        return false;
    }
    memcpy(out_payload, &result, sizeof(result));

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
