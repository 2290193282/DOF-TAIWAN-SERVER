#include "old_data/db/game_option/db_save_game_option_2.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);



typedef char SIG_LOAD_CHARAC_HOTKEY_OPTION_Size_Must_Be_1292[
    (sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION) == 0x50c) ? 1 : -1];

// IDA_EA: 0x844EC26
// IDA_NAME: _ZN19DB_SaveGameOption_2C2Ev
// DEMANGLED: DB_SaveGameOption_2::DB_SaveGameOption_2(void)
DB_SaveGameOption_2::DB_SaveGameOption_2() {}

// IDA_EA: 0x845DD0E
// IDA_NAME: _ZN19DB_SaveGameOption_2D2Ev
// DEMANGLED: DB_SaveGameOption_2::~DB_SaveGameOption_2()
// IDA_EA: 0x845DD3E
// IDA_NAME: _ZN19DB_SaveGameOption_2D0Ev
// DEMANGLED: DB_SaveGameOption_2::~DB_SaveGameOption_2() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGameOption_2::~DB_SaveGameOption_2() {}

// IDA_EA: 0x842FD42
// IDA_NAME: _ZN19DB_SaveGameOption_28dispatchEiiP6Stream
// DEMANGLED: DB_SaveGameOption_2::dispatch(int,int,Stream *)
bool DB_SaveGameOption_2::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    const int bytes = sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION);
    if (!stream->enable_out(bytes)) {
        return false;
    }
    SIG_LOAD_CHARAC_HOTKEY_OPTION* payload =
        reinterpret_cast<SIG_LOAD_CHARAC_HOTKEY_OPTION*>(
            stream->get_buffer());
    stream->out_ptr(bytes);

    const bool is_empty =
        payload->option_length == 0 && payload->hotkey_option.isEmpty();
    if (payload->key_type > 1u ||
        payload->option_length > sizeof(payload->hotkey_option.bytes_) ||
        (!is_empty &&
         !payload->hotkey_option.isValidText(payload->option_length))) {
        return false;
    }

    // Historical zlib wrapper contract: raw + raw/5 + 12.
    char compressed[1548];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     payload->hotkey_option.bytes_,
                     sizeof(payload->hotkey_option.bytes_)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(11),
        static_cast<ENUM_SERVER_GROUP>(0));
    const char* account_id = NumberToString(payload->m_id, 0);
    char* key_blob = db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate member_key_option set key_option='%s' where m_id=%s and key_type=%d",
        key_blob,
        account_id,
        static_cast<int>(payload->key_type));

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        key_blob = db_handle->blob_to_str(0, compressed, compressed_len);
        account_id = NumberToString(payload->m_id, 0);
        db_handle->set_query(
            "inSert into member_key_option(m_id, key_type, key_option) values(%s,%d,'%s')",
            account_id,
            static_cast<int>(payload->key_type),
            key_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
