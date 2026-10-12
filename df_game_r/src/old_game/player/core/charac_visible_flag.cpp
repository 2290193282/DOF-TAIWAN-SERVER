#include "old_game/player/core/charac_visible_flag.h"


// IDA_EA: 0x8695C60
// IDA_NAME: _ZN20tagCharacVisibleFlagC2Ev
// DEMANGLED: tagCharacVisibleFlag::tagCharacVisibleFlag(void)
tagCharacVisibleFlag::tagCharacVisibleFlag()
    : visible_flag(0) {}

// IDA_EA: 0x8695C6C
// IDA_NAME: _ZN20tagCharacVisibleFlag5resetEv
// DEMANGLED: tagCharacVisibleFlag::reset(void)
tagCharacVisibleFlag* tagCharacVisibleFlag::reset() {
    visible_flag = 0;
    return this;
}
