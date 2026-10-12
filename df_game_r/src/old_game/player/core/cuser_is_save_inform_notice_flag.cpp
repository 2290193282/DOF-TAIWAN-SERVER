#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868FA46
// IDA_NAME: _ZN5CUser22isSaveInformNoticeFlagEv
// DEMANGLED: CUser::isSaveInformNoticeFlag(void)
// Active reconstructed-tree ABI: _ZN5CUser22isSaveInformNoticeFlagEv.
int CUser::isSaveInformNoticeFlag() {
    return inform_notice_.save_pending;
}
