#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868FA66
// IDA_NAME: _ZN5CUser27disableSaveInformNoticeFlagEv
// DEMANGLED: CUser::disableSaveInformNoticeFlag(void)
// Active reconstructed-tree ABI: _ZN5CUser27disableSaveInformNoticeFlagEv.
CUser* CUser::disableSaveInformNoticeFlag() {
    inform_notice_.save_pending = 0;
    return this;
}
