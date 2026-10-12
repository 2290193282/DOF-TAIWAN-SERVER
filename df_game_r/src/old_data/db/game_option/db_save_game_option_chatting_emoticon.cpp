#include "old_data/db/game_option/db_save_game_option_chatting_emoticon.h"

#include "old_data/db/core/mysql.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"


extern const char* get_member_game_option()
    __asm__("_Z22get_member_game_optionv");
typedef char SIG_SAVE_CHATTING_EMOTICON_Size_Must_Be_532[
    (sizeof(SIG_SAVE_CHATTING_EMOTICON) == 532) ? 1 : -1];

// IDA_EA: 0x844FB5A
// IDA_NAME: _ZN34DB_SaveGameOption_ChattingEmoticonC2Ev
// DEMANGLED: DB_SaveGameOption_ChattingEmoticon::DB_SaveGameOption_ChattingEmoticon(void)
DB_SaveGameOption_ChattingEmoticon::DB_SaveGameOption_ChattingEmoticon() {}

// IDA_EA: 0x845DBD6
// IDA_NAME: _ZN34DB_SaveGameOption_ChattingEmoticonD2Ev
// DEMANGLED: DB_SaveGameOption_ChattingEmoticon::~DB_SaveGameOption_ChattingEmoticon()
// IDA_EA: 0x845DC06
// IDA_NAME: _ZN34DB_SaveGameOption_ChattingEmoticonD0Ev
// DEMANGLED: DB_SaveGameOption_ChattingEmoticon::~DB_SaveGameOption_ChattingEmoticon() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGameOption_ChattingEmoticon::~DB_SaveGameOption_ChattingEmoticon() {}

// IDA_EA: 0x8430412
// IDA_NAME: _ZN34DB_SaveGameOption_ChattingEmoticon8dispatchEiiP6Stream
// DEMANGLED: DB_SaveGameOption_ChattingEmoticon::dispatch(int,int,Stream *)
bool DB_SaveGameOption_ChattingEmoticon::dispatch(int signal,
                                                  int user_idx,
                                                  Stream* stream) {
    (void)signal;
    (void)user_idx;

    const int bytes = sizeof(SIG_SAVE_CHATTING_EMOTICON);
    if (!stream->enable_out(bytes)) {
        return false;
    }
    SIG_SAVE_CHATTING_EMOTICON* payload =
        reinterpret_cast<SIG_SAVE_CHATTING_EMOTICON*>(stream->get_buffer());
    stream->out_ptr(bytes);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(6),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* option_blob = 0;
    if (put_compressed_blob_data(
            db_handle,
            0,
            reinterpret_cast<char*>(payload->shortcut_emoticon),
            sizeof(payload->shortcut_emoticon),
            &option_blob) != 1) {
        return false;
    }

    const char* account_id = NumberToString(payload->m_id, 0);
    const char* table_name = get_member_game_option();
    db_handle->set_query(
        "upDate %s set shortcut_emoticon='%s' where m_id=%s",
        table_name,
        option_blob,
        account_id);

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        account_id = NumberToString(payload->m_id, 0);
        table_name = get_member_game_option();
        db_handle->set_query(
            "inSert into %s(m_id, shortcut_emoticon) values(%s,'%s')",
            table_name,
            account_id,
            option_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
