#include "old_data/PVF/pvp_mission/combo_script_key.h"

#include "common/config.h"

typedef char combo_script_key_Size_Must_Be_0x4[
    (sizeof(combo_script_key) == 0x4) ? 1 : -1];

// IDA_EA: 0x85E68DE
// IDA_NAME: _ZN16combo_script_keyC2Ev
// DEMANGLED: combo_script_key::combo_script_key(void)
combo_script_key::combo_script_key() { clear(); }

// IDA_EA: 0x8A5A81C
// IDA_NAME: _ZNK16combo_script_keyltERKS_
// DEMANGLED: combo_script_key::operator<(combo_script_key const&)const
bool combo_script_key::operator<(const combo_script_key& rhs) const {
    if (job == rhs.job) {
        return skill_index < rhs.skill_index;
    }
    return job < rhs.job;
}

// IDA_EA: 0x8A5A858
// IDA_NAME: _ZN16combo_script_key5clearEv
// DEMANGLED: combo_script_key::clear(void)
combo_script_key* combo_script_key::clear() {
    job = oldsrv::config::kCharacterJobInvalid;
    skill_index = 0;
    return this;
}
