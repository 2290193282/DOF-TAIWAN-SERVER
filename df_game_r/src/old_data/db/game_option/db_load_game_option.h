#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_LOAD_GAME_OPTION_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_LOAD_GAME_OPTION_H_

#include "old_data/db/core/db_dispatcher.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/game_etc_option_payload.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8452B58
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_REQ_LOAD_GAME_OPTIONEEPT_v
// Old SIG_REQ_LOAD_GAME_OPTION payload size is 4 bytes.
struct SIG_REQ_LOAD_GAME_OPTION {
    uint32_t m_id;
};

// IDA_EA: 0x84568F4
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_LOAD_GAME_OPTIONEEPT_v
// SIG_LOAD_GAME_OPTION remains contiguous and is consumed by
// Inter_LoadGameOption::dispatch_sig @ 0x84D7E84.  The three members are
// cleared and filled independently in DB_LoadGameOption::dispatch @ 0x842F7F4.
struct SIG_LOAD_GAME_OPTION {
    CGameEtcOptionPayload etc_option;
    CQuickChattingOption quick_chat[2];
    CChattingEmoticonConfig emoticon_config;
};
#pragma pack(pop)

static_assert(offsetof(SIG_LOAD_GAME_OPTION, etc_option) == 0x000u,
              "SIG_LOAD_GAME_OPTION ETC option offset drift");
static_assert(offsetof(SIG_LOAD_GAME_OPTION, quick_chat) ==
                  sizeof(CGameEtcOptionPayload),
              "SIG_LOAD_GAME_OPTION quick-chat offset drift");
static_assert(offsetof(SIG_LOAD_GAME_OPTION, emoticon_config) ==
                  sizeof(CGameEtcOptionPayload) +
                      sizeof(CQuickChattingOption) * 2u,
              "SIG_LOAD_GAME_OPTION emoticon offset drift");
static_assert(sizeof(SIG_LOAD_GAME_OPTION) ==
                  sizeof(CGameEtcOptionPayload) +
                      sizeof(CQuickChattingOption) * 2u +
                      sizeof(CChattingEmoticonConfig),
              "SIG_LOAD_GAME_OPTION extent must follow the configured ABI");

// IDA unit: 13_database_dispatch/DB_LoadGameOption.
class DB_LoadGameOption : public DBDispatcher {
public:
    DB_LoadGameOption();
    virtual ~DB_LoadGameOption();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_LOAD_GAME_OPTION_H_
