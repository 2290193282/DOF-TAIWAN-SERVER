#include "old_game/player/core/character_load_mercenary.h"

#include <cstring>


typedef char CHARAC_LOAD_MERCENARY_Size_Must_Be_0x54[
    (sizeof(CHARAC_LOAD_MERCENARY) == 0x54) ? 1 : -1];

// IDA_EA: 0x81B410A
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY5ResetEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::Reset(void)
CHARAC_LOAD_MERCENARY* CHARAC_LOAD_MERCENARY::Reset() {
    charac_no = 0;
    level = 0;
    std::memset(name, 0, sizeof(name));
    std::memset(avatar_item_id, 0, sizeof(avatar_item_id));
    save_flag = 0;
    start_time = 0;
    end_time = 0;
    competition_area = 0xffu;
    competition_period = 0xffu;
    avatar_bonus_index = 10;
    return this;
}

// IDA_EA: 0x81B4190
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY18SendMessageToCUserEP5CUserPc
// DEMANGLED: CHARAC_LOAD_MERCENARY::SendMessageToCUser(CUser *,char *)const
void CHARAC_LOAD_MERCENARY::SendMessageToCUser(CUser* user,
                                               char* message) const {
    (void)user;
    (void)message;
}

// IDA_EA: 0x81B47C8
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY5alterEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::alter(void)
CHARAC_LOAD_MERCENARY* CHARAC_LOAD_MERCENARY::alter() {
    save_flag = 1;
    return this;
}

// IDA_EA: 0x81B52F4
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARYD2Ev
// DEMANGLED: CHARAC_LOAD_MERCENARY::~CHARAC_LOAD_MERCENARY()
CHARAC_LOAD_MERCENARY::~CHARAC_LOAD_MERCENARY() {}

// IDA_EA: 0x8694C08
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARYC2Ev
// DEMANGLED: CHARAC_LOAD_MERCENARY::CHARAC_LOAD_MERCENARY(void)
CHARAC_LOAD_MERCENARY::CHARAC_LOAD_MERCENARY() {
    charac_no = 0;
    level = 0;
    save_flag = 0;
    start_time = 0;
    end_time = 0;
    competition_area = 0xffu;
    competition_period = 0xffu;
    avatar_bonus_index = 10;
    std::memset(name, 0, sizeof(name));
    std::memset(avatar_item_id, 0, sizeof(avatar_item_id));
}

// IDA_EA: 0x8694C8E
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY13save_completeEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::save_complete(void)
CHARAC_LOAD_MERCENARY* CHARAC_LOAD_MERCENARY::save_complete() {
    save_flag = 0;
    return this;
}
