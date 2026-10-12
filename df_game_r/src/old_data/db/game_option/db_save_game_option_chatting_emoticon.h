#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_CHATTING_EMOTICON_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_CHATTING_EMOTICON_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8452D12
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_SAVE_CHATTING_EMOTICONEEPT_v
// Old SIG_SAVE_CHATTING_EMOTICON payload size is 532 bytes.
struct SIG_SAVE_CHATTING_EMOTICON {
    uint32_t m_id;
    uint8_t shortcut_emoticon[528];
};
#pragma pack(pop)

// IDA unit: 13_database_dispatch/DB_SaveGameOption_ChattingEmoticon.
class DB_SaveGameOption_ChattingEmoticon : public DBDispatcher {
public:
    DB_SaveGameOption_ChattingEmoticon();
    virtual ~DB_SaveGameOption_ChattingEmoticon();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_CHATTING_EMOTICON_H_
