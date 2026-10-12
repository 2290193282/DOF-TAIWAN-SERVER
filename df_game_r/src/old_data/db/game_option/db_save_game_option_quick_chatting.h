#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_QUICK_CHATTING_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_QUICK_CHATTING_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8452CBE
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_SAVE_GAME_OPTION_QUICKCHATEEPT_v
// Old SIG_SAVE_GAME_OPTION_QUICKCHAT payload size is 604 bytes.
struct SIG_SAVE_GAME_OPTION_QUICKCHAT {
    uint32_t m_id;
    uint8_t option_3[600];
};
#pragma pack(pop)

// IDA unit: 13_database_dispatch/DB_SaveGameOption_QuickChatting.
class DB_SaveGameOption_QuickChatting : public DBDispatcher {
public:
    DB_SaveGameOption_QuickChatting();
    virtual ~DB_SaveGameOption_QuickChatting();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_QUICK_CHATTING_H_
