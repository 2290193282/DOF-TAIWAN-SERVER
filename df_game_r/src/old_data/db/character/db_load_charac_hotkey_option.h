#ifndef OLDSRV_DB_DB_LOAD_CHARAC_HOTKEY_OPTION_H_
#define OLDSRV_DB_DB_LOAD_CHARAC_HOTKEY_OPTION_H_

#include "old_data/db/core/db_dispatcher.h"
#include "common/manager/common_time_game_option_abi.h"

#include <stddef.h>
#include <stdint.h>


// IDA_EA: 0x8452AEE
// IDA_NAME: _ZN6Stream12GetOutBufferI33SIG_REQ_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// Old SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION payload size is 8 bytes.
struct SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION {
    uint32_t m_id;
    uint32_t key_type;
};

// IDA_EA: 0x8452BC2
// IDA_NAME: _ZN6Stream12GetOutBufferI29SIG_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// 140828 load/save signal: 12-byte metadata followed by the 1280-byte text
// storage object.  Its queue/memberization extent is therefore 1292 bytes.
struct SIG_LOAD_CHARAC_HOTKEY_OPTION {
    uint32_t m_id;
    uint8_t key_type;
    uint8_t reserved_05[3];
    uint32_t option_length;
    CHotkeyOption hotkey_option;
};

static_assert(sizeof(SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION) == 8,
              "SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION ABI drift");
static_assert(offsetof(SIG_LOAD_CHARAC_HOTKEY_OPTION, option_length) == 8,
              "hotkey signal length offset drift");
static_assert(offsetof(SIG_LOAD_CHARAC_HOTKEY_OPTION, hotkey_option) == 12,
              "hotkey signal text offset drift");
static_assert(sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION) == 0x50c,
              "SIG_LOAD_CHARAC_HOTKEY_OPTION ABI drift");

// IDA unit: 04_character_select_create/DB_LoadCharacHotKeyOption.
class DB_LoadCharacHotKeyOption : public DBDispatcher {
public:
    DB_LoadCharacHotKeyOption();
    virtual ~DB_LoadCharacHotKeyOption();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA 0x842F710 receives exactly three stack arguments.  Although the
    // exported name is class-qualified, this is a static request builder:
    // its first argument is the packed CUser UID, not an object pointer.
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int key_type);
};

#endif  // OLDSRV_DB_DB_LOAD_CHARAC_HOTKEY_OPTION_H_
