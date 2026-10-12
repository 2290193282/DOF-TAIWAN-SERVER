#include "old_game/player/core/cuser.h"

// IDA_EA: 0x868DC7A
// IDA_NAME: _ZN5CUser14alonePlay_ModeEv
// DEMANGLED: CUser::alonePlay_Mode(void)
bool CUser::alonePlay_Mode() {
    return GetGameOptionRef()->get_etc_option(0x1b) == 1;
}
