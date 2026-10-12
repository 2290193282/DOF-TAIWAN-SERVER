#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868FA76
// IDA_NAME: _ZN5CUser19getInformNoticeFlagEi
// DEMANGLED: CUser::getInformNoticeFlag(int)
// Active reconstructed-tree ABI: _ZN5CUser19getInformNoticeFlagEi.
int CUser::getInformNoticeFlag(int index) {
    return inform_notice_.flags[static_cast<unsigned int>(index)];
}
