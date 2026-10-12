#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_1_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_1_H_

#include "old_data/db/core/db_dispatcher.h"
#include "common/manager/game_etc_option_payload.h"

#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8452C16
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_SAVE_GAME_OPTION_1EEPT_v
// Internal save request. option_1 is compressed into the existing
// member_game_option.option_1 BLOB.
struct SIG_SAVE_GAME_OPTION_1 {
    uint32_t m_id;
    CGameEtcOptionPayload option_1;
};
#pragma pack(pop)

// IDA unit: 13_database_dispatch/DB_SaveGameOption_1.
class DB_SaveGameOption_1 : public DBDispatcher {
public:
    DB_SaveGameOption_1();
    virtual ~DB_SaveGameOption_1();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_1_H_
