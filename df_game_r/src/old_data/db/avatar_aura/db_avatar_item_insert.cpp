#include "old_data/db/avatar_aura/db_avatar_item_insert.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


extern int GetIdentityFromDB(MySQL* mysql)
    __asm__("_Z17GetIdentityFromDBP5MySQL");

namespace {

enum {
    kDbSignalAvatarItemInsertResult = 49,
};

}  // namespace

// IDA_EA: 0x850D3C8
// IDA_NAME: _ZN17SIG_AVATAR_INSERT3setEijiibcPKcim
// DEMANGLED: SIG_AVATAR_INSERT::set(int,unsigned int,int,int,bool,char,char const*,int,unsigned long)
SIG_AVATAR_INSERT* SIG_AVATAR_INSERT::set(
    int charac_no_value,
    unsigned int item_id_value,
    int slot_value,
    int expire_time_value,
    bool obtain_from_value,
    char ability_no_value,
    const char* ipg_agency_no_value,
    int stat_value,
    unsigned long add_info_value) {
    charac_no = charac_no_value;
    item_id = item_id_value;
    slot = static_cast<uint8_t>(slot_value);
    expire_time = expire_time_value;
    obtain_from = obtain_from_value;
    ability_no = ability_no_value;
    if (ipg_agency_no_value) {
        strncpy(ipg_agency_no, ipg_agency_no_value, sizeof(ipg_agency_no));
    }
    add_info = static_cast<uint32_t>(add_info_value);
    stat = static_cast<int8_t>(stat_value);
    return this;
}

// IDA_EA: 0x850D44C
// IDA_NAME: _ZN17SIG_AVATAR_INSERT14setJewelSocketERK20stAvatarEmblemInfo_t
// DEMANGLED: SIG_AVATAR_INSERT::setJewelSocket(stAvatarEmblemInfo_t const&)
SIG_AVATAR_INSERT* SIG_AVATAR_INSERT::setJewelSocket(
    const stAvatarEmblemInfo_t& source) {
    memcpy(&jewel_socket, &source, sizeof(jewel_socket));
    return this;
}

// IDA_EA: 0x83FFD50
// IDA_NAME: _ZN19DB_AvatarItemInsertC2Ev
// DEMANGLED: DB_AvatarItemInsert::DB_AvatarItemInsert(void)
DB_AvatarItemInsert::DB_AvatarItemInsert() {}

// IDA_EA: 0x83FFD6C
// IDA_NAME: _ZN19DB_AvatarItemInsertD2Ev
// DEMANGLED: DB_AvatarItemInsert::~DB_AvatarItemInsert()
// IDA_EA: 0x83FFD9C
// IDA_NAME: _ZN19DB_AvatarItemInsertD0Ev
// DEMANGLED: DB_AvatarItemInsert::~DB_AvatarItemInsert() deleting destructor
DB_AvatarItemInsert::~DB_AvatarItemInsert() {}

// IDA_EA: 0x83FFDBA
// IDA_NAME: _ZN19DB_AvatarItemInsert8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarItemInsert::dispatch(int,int,Stream *)
bool DB_AvatarItemInsert::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;

    SIG_AVATAR_INSERT* payload = stream->GetOutBuffer<SIG_AVATAR_INSERT>();
    if (!payload) {
        return false;
    }

    if (InsertAvatarItem(payload) != true) {
        return false;
    }
    _SendResult(payload, user_idx);
    return true;
}

// IDA_EA: 0x83FFE0E
// IDA_NAME: _ZN19DB_AvatarItemInsert16InsertAvatarItemEP17SIG_AVATAR_INSERT
// DEMANGLED: DB_AvatarItemInsert::InsertAvatarItem(SIG_AVATAR_INSERT *)
bool DB_AvatarItemInsert::InsertAvatarItem(SIG_AVATAR_INSERT* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char date_text[64];
    memset(date_text, 0, sizeof(date_text));
    strcpy(date_text, "9999-12-31 23:59:59");

    const char* format =
        "inSert into user_items (charac_no,slot,it_id,expire_date,obtain_from,reg_date,ability_no,ipg_agency_no,stat,jewel_socket,hidden_option,emblem_endurance) values(%u,%d,%u,'%s',%d,now(),%d,'%s',%d,'%s',%d,%d)";
    const char* unix_format =
        "inSert into user_items (charac_no,slot,it_id,expire_date,obtain_from,reg_date,ability_no,ipg_agency_no,stat,jewel_socket,hidden_option,emblem_endurance) values(%u,%d,%u,from_unixtime('%s'),%d,now(),%d,'%s',%d,'%s',%d,%d)";

    if (payload->expire_time) {
        format = unix_format;
        memset(date_text, 0, sizeof(date_text));
        const int expire_time = payload->expire_time;
        cMyTrace trace(
            "static bool DB_AvatarItemInsert::InsertAvatarItem(SIG_AVATAR_INSERT*)",
            2785,
            0);
        trace("[InsertAvatarItem] pContext->nExpireTime : %d", expire_time);
        if (expire_time > 30 || expire_time <= 6) {
            snprintf(date_text, sizeof(date_text), "%d", expire_time);
            cMyTrace period_trace(
                "static bool DB_AvatarItemInsert::InsertAvatarItem(SIG_AVATAR_INSERT*)",
                2794,
                0);
            period_trace("[InsertAvatarItem] period value : %s", date_text);
        } else {
            CSystemTime* system_time = GlobalData::system_time();
            const int now = system_time ? system_time->getCurSec() : 0;
            snprintf(date_text, sizeof(date_text), "%d",
                     86400 * expire_time + now);
            cMyTrace period_trace(
                "static bool DB_AvatarItemInsert::InsertAvatarItem(SIG_AVATAR_INSERT*)",
                2789,
                0);
            period_trace("[InsertAvatarItem] period value : %s", date_text);
        }
    }

    char* jewel_socket = db_handle->blob_to_str(
        0, &payload->jewel_socket, sizeof(payload->jewel_socket));
    db_handle->set_query(format,
                         payload->charac_no,
                         payload->slot,
                         payload->item_id,
                         date_text,
                         payload->obtain_from,
                         static_cast<unsigned char>(payload->ability_no),
                         payload->ipg_agency_no,
                         payload->stat,
                         jewel_socket,
                         static_cast<uint16_t>(payload->add_info),
                         static_cast<uint16_t>(payload->add_info >> 16),
                         static_cast<int16_t>(
                             payload->expansion_info.avatar_expansion[0]),
                         static_cast<int16_t>(
                             payload->expansion_info.avatar_expansion[1]));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84001FE
// IDA_NAME: _ZN19DB_AvatarItemInsert11_SendResultEP17SIG_AVATAR_INSERTi
// DEMANGLED: DB_AvatarItemInsert::_SendResult(SIG_AVATAR_INSERT *,int)
void DB_AvatarItemInsert::_SendResult(SIG_AVATAR_INSERT* payload,
                                      int user_idx) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 2840);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalAvatarItemInsertResult;
    *guard << user_idx;
    SIG_AVATAR_INSERT* result =
        guard.GetInBuffer<SIG_AVATAR_INSERT>();
    if (!result) {
        return;
    }
    memcpy(result, payload, sizeof(*result));
    result->ui_id = static_cast<uint32_t>(GetIdentityFromDB(db_handle));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}
