#include "old_data/db/game_option/db_save_game_option_quick_chatting.h"

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

typedef char SIG_SAVE_GAME_OPTION_QUICKCHAT_Size_Must_Be_604[
    (sizeof(SIG_SAVE_GAME_OPTION_QUICKCHAT) == 604) ? 1 : -1];

// IDA_EA: 0x844F5FE
// IDA_NAME: _ZN31DB_SaveGameOption_QuickChattingC2Ev
// DEMANGLED: DB_SaveGameOption_QuickChatting::DB_SaveGameOption_QuickChatting(void)
DB_SaveGameOption_QuickChatting::DB_SaveGameOption_QuickChatting() {}

// IDA_EA: 0x845DC24
// IDA_NAME: _ZN31DB_SaveGameOption_QuickChattingD2Ev
// DEMANGLED: DB_SaveGameOption_QuickChatting::~DB_SaveGameOption_QuickChatting()
// IDA_EA: 0x845DC54
// IDA_NAME: _ZN31DB_SaveGameOption_QuickChattingD0Ev
// DEMANGLED: DB_SaveGameOption_QuickChatting::~DB_SaveGameOption_QuickChatting() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGameOption_QuickChatting::~DB_SaveGameOption_QuickChatting() {}

// IDA_EA: 0x8430250
// IDA_NAME: _ZN31DB_SaveGameOption_QuickChatting8dispatchEiiP6Stream
// DEMANGLED: DB_SaveGameOption_QuickChatting::dispatch(int,int,Stream *)
bool DB_SaveGameOption_QuickChatting::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;

    const int bytes = sizeof(SIG_SAVE_GAME_OPTION_QUICKCHAT);
    if (!stream->enable_out(bytes)) {
        return false;
    }
    SIG_SAVE_GAME_OPTION_QUICKCHAT* payload =
        reinterpret_cast<SIG_SAVE_GAME_OPTION_QUICKCHAT*>(
            stream->get_buffer());
    stream->out_ptr(bytes);

    char compressed[732];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(payload->option_3),
                     sizeof(payload->option_3)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(6),
        static_cast<ENUM_SERVER_GROUP>(0));
    const char* account_id = NumberToString(payload->m_id, 0);
    char* option_blob = db_handle->blob_to_str(1,
                                               compressed,
                                               compressed_len);
    const char* table_name = get_member_game_option();
    db_handle->set_query("upDate %s set option_3='%s' where m_id=%s",
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
            "inSert into %s(m_id, option_3) values(%s,'%s')",
            table_name,
            account_id,
            option_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
