#include "old_data/db/game_option/db_save_game_option_1.h"

#include "common/config.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);


extern const char* get_member_game_option()
    __asm__("_Z22get_member_game_optionv");

typedef char SIG_SAVE_GAME_OPTION_1_Size_Must_Follow_Config[
    (sizeof(SIG_SAVE_GAME_OPTION_1) ==
     sizeof(uint32_t) + oldsrv::config::kGameEtcOptionBytes) ? 1 : -1];

// IDA_EA: 0x844EC0A
// IDA_NAME: _ZN19DB_SaveGameOption_1C2Ev
// DEMANGLED: DB_SaveGameOption_1::DB_SaveGameOption_1(void)
DB_SaveGameOption_1::DB_SaveGameOption_1() {}

// IDA_EA: 0x845DCC0
// IDA_NAME: _ZN19DB_SaveGameOption_1D2Ev
// DEMANGLED: DB_SaveGameOption_1::~DB_SaveGameOption_1()
// IDA_EA: 0x845DCF0
// IDA_NAME: _ZN19DB_SaveGameOption_1D0Ev
// DEMANGLED: DB_SaveGameOption_1::~DB_SaveGameOption_1() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGameOption_1::~DB_SaveGameOption_1() {}

// IDA_EA: 0x842FF4E
// IDA_NAME: _ZN19DB_SaveGameOption_18dispatchEiiP6Stream
// DEMANGLED: DB_SaveGameOption_1::dispatch(int,int,Stream *)
bool DB_SaveGameOption_1::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    const int bytes = sizeof(SIG_SAVE_GAME_OPTION_1);
    if (!stream->enable_out(bytes)) {
        return false;
    }
    SIG_SAVE_GAME_OPTION_1* payload =
        reinterpret_cast<SIG_SAVE_GAME_OPTION_1*>(stream->get_buffer());
    stream->out_ptr(bytes);

    char compressed[oldsrv::config::kMaximumCompressedGameEtcOptionBytes];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(&payload->option_1),
                     sizeof(payload->option_1)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(6),
        static_cast<ENUM_SERVER_GROUP>(0));
    const char* account_id = NumberToString(payload->m_id, 0);
    char* option_blob = db_handle->blob_to_str(0,
                                               compressed,
                                               compressed_len);
    const char* table_name = get_member_game_option();
    db_handle->set_query("upDate %s set option_1='%s' where m_id=%s",
                         table_name,
                         option_blob,
                         account_id);

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        option_blob = db_handle->blob_to_str(0, compressed, compressed_len);
        account_id = NumberToString(payload->m_id, 0);
        table_name = get_member_game_option();
        db_handle->set_query(
            "inSert into %s(m_id, option_1) values(%s,'%s')",
            table_name,
            account_id,
            option_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
