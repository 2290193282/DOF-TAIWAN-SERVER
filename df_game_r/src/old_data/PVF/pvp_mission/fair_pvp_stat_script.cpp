#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"


// IDA_EA: 0x8568652
// IDA_NAME: _ZN13CharacterKindC2Ev
// DEMANGLED: CharacterKind::CharacterKind(void)
CharacterKind::CharacterKind()
    : charac_job(0),
      first_grow_type(0) {}

// IDA_EA: 0x8693BF4
// IDA_NAME: _ZNK13CharacterKindltERKS_
// DEMANGLED: CharacterKind::operator<(CharacterKind const&) const
bool CharacterKind::operator<(const CharacterKind& rhs) const {
    if (charac_job < rhs.charac_job) {
        return true;
    }
    return charac_job == rhs.charac_job &&
           first_grow_type < rhs.first_grow_type;
}

// IDA_EA: 0x8693C3E
// IDA_NAME: _ZN13CharacterKindC2Ehh
// DEMANGLED: CharacterKind::CharacterKind(unsigned char,unsigned char)
CharacterKind::CharacterKind(uint8_t charac_job_value,
                             uint8_t first_grow_type_value)
    : charac_job(charac_job_value),
      first_grow_type(first_grow_type_value) {}

// IDA_EA: 0x83724C8
// IDA_NAME: _ZN17FairPvPStatScript5clearEv
// DEMANGLED: FairPvPStatScript::clear(void)
void FairPvPStatScript::clear() {
    character_stats.clear();
}

// IDA_EA: 0x83724F0
// IDA_NAME: _ZN17FairPvPStatScriptC2Ev
// DEMANGLED: FairPvPStatScript::FairPvPStatScript(void)
FairPvPStatScript::FairPvPStatScript()
    : character_stats() {
    clear();
}

// IDA_EA: 0x837C9D4
// IDA_NAME: _ZN17FairPvPStatScriptD2Ev
// DEMANGLED: FairPvPStatScript::~FairPvPStatScript()
FairPvPStatScript::~FairPvPStatScript() {}

// IDA_EA: 0x8693C66
// IDA_NAME: _ZN17FairPvPStatScript14FindCharacStatEii
// DEMANGLED: FairPvPStatScript::FindCharacStat(int,int)
CharacterStatus* FairPvPStatScript::FindCharacStat(
    int charac_job_value,
    int first_grow_type_value) {
    const CharacterKind kind(static_cast<uint8_t>(charac_job_value),
                             static_cast<uint8_t>(first_grow_type_value));
    std::map<CharacterKind, CharacterStatus>::iterator it =
        character_stats.find(kind);
    if (it != character_stats.end()) {
        return &it->second;
    }
    return 0;
}
