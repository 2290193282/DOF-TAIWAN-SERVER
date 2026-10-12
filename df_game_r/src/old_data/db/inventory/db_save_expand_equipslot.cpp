#include "old_data/db/inventory/db_save_expand_equipslot.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream_expand_equipslot.h"

#include <stdint.h>
#include <string.h>

namespace {

enum {
    kSaveExpandEquipslotCompressedBytes = 2652,
};

}  // namespace

// IDA_EA: 0x844F6DE
// IDA_NAME: _ZN22DB_SaveExpandEquipslotC2Ev
// DEMANGLED: DB_SaveExpandEquipslot::DB_SaveExpandEquipslot(void)
DB_SaveExpandEquipslot::DB_SaveExpandEquipslot() : DBDispatcher() {}

// IDA_EA: 0x845FB38
// IDA_NAME: _ZN22DB_SaveExpandEquipslotD2Ev
// DEMANGLED: DB_SaveExpandEquipslot::~DB_SaveExpandEquipslot()
// IDA_EA: 0x845FB68
// IDA_NAME: _ZN22DB_SaveExpandEquipslotD0Ev
// DEMANGLED: DB_SaveExpandEquipslot::~DB_SaveExpandEquipslot()
DB_SaveExpandEquipslot::~DB_SaveExpandEquipslot() {}

// IDA_EA: 0x840DA96
// IDA_NAME: _ZN22DB_SaveExpandEquipslot8dispatchEiiP6Stream
// DEMANGLED: DB_SaveExpandEquipslot::dispatch(int,int,Stream *)
bool DB_SaveExpandEquipslot::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_EXPAND_CHARAC* const payload =
        stream->GetOutBuffer<SIG_SAVE_EXPAND_CHARAC>();
    return _SaveExpandEquipslot(&payload->equipslot) == true &&
           _SaveCharacStat(payload->equipslot.charac_no,
                           &payload->stat) == true;
}

// IDA_EA: 0x843FEBE
// IDA_NAME: _ZN22DB_SaveExpandEquipslot15_SaveCharacStatEjP27SIG_SAVE_EXPAND_CHARAC_STAT
// DEMANGLED: DB_SaveExpandEquipslot::_SaveCharacStat(unsigned int,SIG_SAVE_EXPAND_CHARAC_STAT *)
bool DB_SaveExpandEquipslot::_SaveCharacStat(unsigned int charac_no,
                                             SIG_SAVE_EXPAND_CHARAC_STAT* stat_payload) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate charac_stat set channel_equipslot_switch=%d, expand_equipslot_switch=%d where charac_no = %u",
        stat_payload->channel_equipslot_switch,
        stat_payload->expand_equipslot_switch,
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x843FF48
// IDA_NAME: _ZN22DB_SaveExpandEquipslot20_SaveExpandEquipslotEP25SIG_SAVE_EXPAND_EQUIPSLOT
// DEMANGLED: DB_SaveExpandEquipslot::_SaveExpandEquipslot(SIG_SAVE_EXPAND_EQUIPSLOT *)
bool DB_SaveExpandEquipslot::_SaveExpandEquipslot(SIG_SAVE_EXPAND_EQUIPSLOT* payload_data) {
    char compressed[kSaveExpandEquipslotCompressedBytes];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(payload_data),
                     sizeof(*payload_data)) != 1) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* expand_blob = db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate charac_inven_expand set expand_equipslot='%s' where charac_no=%u",
        expand_blob,
        payload_data->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    expand_blob = db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "inSert into charac_inven_expand(charac_no, expand_equipslot) values(%u, '%s')",
        payload_data->charac_no,
        expand_blob);
    return db_handle->exec(true) == true;
}
