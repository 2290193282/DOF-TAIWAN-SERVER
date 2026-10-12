#include "old_data/db/inventory/db_save_item_dictionary_flag.h"

#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844FA7A
// IDA_NAME: _ZN25DB_SaveItemDictionaryFlagC2Ev
// DEMANGLED: DB_SaveItemDictionaryFlag::DB_SaveItemDictionaryFlag(void)
DB_SaveItemDictionaryFlag::DB_SaveItemDictionaryFlag() {}

// IDA_EA: 0x845B4D6
// IDA_NAME: _ZN25DB_SaveItemDictionaryFlagD2Ev
// DEMANGLED: DB_SaveItemDictionaryFlag::~DB_SaveItemDictionaryFlag()
// IDA_EA: 0x845B506
// IDA_NAME: _ZN25DB_SaveItemDictionaryFlagD0Ev
// DEMANGLED: DB_SaveItemDictionaryFlag::~DB_SaveItemDictionaryFlag()
DB_SaveItemDictionaryFlag::~DB_SaveItemDictionaryFlag() {}

// IDA_EA: 0x844547E
// IDA_NAME: _ZN25DB_SaveItemDictionaryFlag8dispatchEiiP6Stream
// DEMANGLED: DB_SaveItemDictionaryFlag::dispatch(int,int,Stream *)
bool DB_SaveItemDictionaryFlag::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_ITEM_DICTIONARY_FLAG* payload =
        stream->GetOutBuffer<SIG_ITEM_DICTIONARY_FLAG>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* compressed_blob = 0;
    if (put_compressed_blob_data(
            db_handle,
            0,
            reinterpret_cast<char*>(payload->flags),
            sizeof(payload->flags),
            &compressed_blob) != 1) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SaveItemDictionaryFlag::dispatch(int, int, Stream*)",
            0xAC56,
            "DB_SaveItemDictionaryFalg::dispatch compress_zip(%u) ERROR charac_no=%u",
            0,
            payload->charac_no);
        return false;
    }

    db_handle->set_query(
        "upDate charac_item_stat set check_flag = '%s' where charac_no=%u",
        compressed_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SaveItemDictionaryFlag::dispatch(int, int, Stream*)",
            0xAC5E,
            "DB_SaveItemDictionaryFalg->upDate Query ERROR charac_no=%u",
            payload->charac_no);
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_item_stat(charac_no, check_flag) values(%u, '%s')",
        payload->charac_no,
        compressed_blob);
    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_SaveItemDictionaryFlag::dispatch(int, int, Stream*)",
        0xAC67,
        "DB_SaveItemDictionaryFalg->inSert Query ERROR charac_no=%u",
        payload->charac_no);
    return false;
}
