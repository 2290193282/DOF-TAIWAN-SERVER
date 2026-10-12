#include "old_game/player/setting/character_option.h"

#include <cstring>


typedef char CharacterOption_Size_Must_Be_0x200[
    (sizeof(CharacterOption) == 0x200) ? 1 : -1];

// IDA_EA: 0x822ABEC
// IDA_NAME: _ZN15CharacterOptionC2Ev
// DEMANGLED: CharacterOption::CharacterOption(void)
CharacterOption::CharacterOption() {
    std::memset(raw_00, 0xff, sizeof(raw_00));
}
