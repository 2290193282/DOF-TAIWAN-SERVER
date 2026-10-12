#include "old_game/player/core/character_db_payloads.h"

#include <cstring>


typedef char CHARAC_DATA_Size_Must_Be_0x6bc[
    (sizeof(CHARAC_DATA) == 0x6bc) ? 1 : -1];
typedef char SIG_LOAD_CHARAC_Size_Must_Be_0x740[
    (sizeof(SIG_LOAD_CHARAC) == 0x740) ? 1 : -1];

// IDA_EA: 0x822EC4C
// IDA_NAME: _ZN11CHARAC_DATAC2Ev
// DEMANGLED: CHARAC_DATA::CHARAC_DATA(void)
CHARAC_DATA::CHARAC_DATA() : additional_info() {
    Reset();
}

// IDA_EA: 0x822EC9E
// IDA_NAME: _ZN11CHARAC_DATA5ResetEv
// DEMANGLED: CHARAC_DATA::Reset(void)
CHARAC_DATA* CHARAC_DATA::Reset() {
    additional_info.clear();
    std::memset(character_name, 0, sizeof(character_name));
    std::memset(create_time, 0, sizeof(create_time));
    std::memset(equipment, 0, sizeof(equipment));
    std::memset(escalade_tutorial_flag, 0,
                sizeof(escalade_tutorial_flag));
    guild_secede = 0;
    add_equipslot_flag = 0;
    channel_equipslot_switch = 0;
    expand_equipslot_switch = 0;
    event_bring_up_reward_flag = 0;
    event_bring_up_material_count = 0;
    growth_power_reward = 0;
    chaos_response_time = 0;
    return this;
}

// IDA_EA: 0x8491C62
// IDA_NAME: _ZN15SIG_LOAD_CHARACC2Ev
// DEMANGLED: SIG_LOAD_CHARAC::SIG_LOAD_CHARAC(void)
SIG_LOAD_CHARAC::SIG_LOAD_CHARAC() {}
