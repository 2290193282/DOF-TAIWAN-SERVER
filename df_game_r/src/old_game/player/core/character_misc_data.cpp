#include "old_game/player/core/character_misc_data.h"


typedef char TradeSlot_Size_Must_Be_0x48[
    (sizeof(TradeSlot) == 0x48) ? 1 : -1];
typedef char Character_Size_Must_Be_0x40[
    (sizeof(Character) == 0x40) ? 1 : -1];
typedef char CharacterEtcStats_Size_Must_Be_0x1c[
    (sizeof(CharacterEtcStats) == 0x1c) ? 1 : -1];
typedef char stCharacLinkSystemData_Size_Must_Be_0x10[
    (sizeof(stCharacLinkSystemData) == 0x10) ? 1 : -1];
typedef char stCharacLinkSystem_t_Size_Must_Be_0x34[
    (sizeof(stCharacLinkSystem_t) == 0x34) ? 1 : -1];
typedef char stQuestTargetCharacter_t_Size_Must_Be_0x0c[
    (sizeof(stQuestTargetCharacter_t) == 0x0c) ? 1 : -1];

// IDA_EA: 0x84867EE
// IDA_NAME: _ZN9CharacterpLERKS_
// DEMANGLED: Character::operator+=(Character const&)
Character& Character::operator+=(const Character& rhs) {
    PlayInfo::operator+=(rhs);
    ++dword_3c;
    return *this;
}

// IDA_EA: 0x8486818
// IDA_NAME: _ZN9Character5ResetEv
// DEMANGLED: Character::Reset(void)
Character* Character::Reset() {
    PlayInfo::Reset();
    dword_3c = 0;
    return this;
}

// IDA_EA: 0x848683C
// IDA_NAME: _ZN9CharacterC2Ev
// DEMANGLED: Character::Character(void)
Character::Character()
    : IData(), PlayInfo(), dword_3c(0) {
    Reset();
}

// IDA_EA: 0x8486866
// IDA_NAME: _ZN9CharacterD2Ev
// DEMANGLED: Character::~Character()
Character::~Character() {}

// IDA_EA: 0x8367752
// IDA_NAME: _ZN17CharacterEtcStats5clearEv
// DEMANGLED: CharacterEtcStats::clear(void)
bool CharacterEtcStats::clear() {
    bool result = false;
    values.clear();
    for (int index = 0;; ++index) {
        result = index <= 3;
        if (index > 3) {
            break;
        }
        raw_0c[index] = 0;
    }
    return result;
}

// IDA_EA: 0x836778C
// IDA_NAME: _ZN17CharacterEtcStatsC2Ev
// DEMANGLED: CharacterEtcStats::CharacterEtcStats(void)
CharacterEtcStats::CharacterEtcStats()
    : values() {
    clear();
}

// IDA_EA: 0x836788C
// IDA_NAME: _ZN17CharacterEtcStatsD2Ev
// DEMANGLED: CharacterEtcStats::~CharacterEtcStats()
CharacterEtcStats::~CharacterEtcStats() {}

// IDA_EA: 0x891B762
// IDA_NAME: _ZN22stCharacLinkSystemDataC2Ev
// DEMANGLED: stCharacLinkSystemData::stCharacLinkSystemData(void)
stCharacLinkSystemData::stCharacLinkSystemData()
    : minimum_character_level(0),
      maximum_character_level(0),
      bonus_rate_percent(0),
      bonus_value(0) {}

// IDA_EA: 0x893C682
// IDA_NAME: _ZN22stCharacLinkSystemDataD2Ev
// DEMANGLED: stCharacLinkSystemData::~stCharacLinkSystemData()
stCharacLinkSystemData::~stCharacLinkSystemData() {}

// IDA_EA: 0x837CF46
// IDA_NAME: _ZN20stCharacLinkSystem_tC2Ev
// DEMANGLED: stCharacLinkSystem_t::stCharacLinkSystem_t(void)
stCharacLinkSystem_t::stCharacLinkSystem_t()
    : striker_skill_minimum_level(0),
      low_link_character_exp_bonus_table(),
      low_link_character_gold_bonus_table(),
      high_link_character_exp_bonus_table(),
      high_link_character_gold_bonus_table() {}

// IDA_EA: 0x837CFDE
// IDA_NAME: _ZN20stCharacLinkSystem_tD2Ev
// DEMANGLED: stCharacLinkSystem_t::~stCharacLinkSystem_t()
stCharacLinkSystem_t::~stCharacLinkSystem_t() {}

// IDA_EA: 0x8A65FC6
// IDA_NAME: _ZN24stQuestTargetCharacter_tC2E17ENUM_CHARACTERJOBii
// DEMANGLED: stQuestTargetCharacter_t::stQuestTargetCharacter_t(ENUM_CHARACTERJOB,int,int)
stQuestTargetCharacter_t::stQuestTargetCharacter_t(int character_job_value,
                                                   int grow_type_value,
                                                   int level_value)
    : character_job(character_job_value),
      grow_type(grow_type_value),
      level(level_value) {}

// IDA_EA: 0x8A65FE6
// IDA_NAME: _ZN24stQuestTargetCharacter_teqERKS_
// DEMANGLED: stQuestTargetCharacter_t::operator==(stQuestTargetCharacter_t const&)
bool stQuestTargetCharacter_t::operator==(
    const stQuestTargetCharacter_t& other) const {
    return character_job == other.character_job &&
           grow_type == other.grow_type &&
           level == other.level;
}

// IDA_EA: 0x8A6956A
// IDA_NAME: _ZN24stQuestTargetCharacter_tD2Ev
// DEMANGLED: stQuestTargetCharacter_t::~stQuestTargetCharacter_t()
stQuestTargetCharacter_t::~stQuestTargetCharacter_t() {}
