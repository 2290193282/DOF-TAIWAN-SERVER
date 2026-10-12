#include "old_game/player/core/cuser.h"

#include "core/config/env/environment.h"

// IDA_EA: 0x868B4FC
// IDA_NAME: _ZN5CUser29getCharacSlotCountDefaultInfoEv
// DEMANGLED: CUser::getCharacSlotCountDefaultInfo(void)
int CUser::getCharacSlotCountDefaultInfo() {
    CEnvironment* const environment = G_CEnvironment();
    const int server_type = environment->get_server_type();
    if (server_type == 1) {
        return 1;
    }
    if (server_type == 2) {
        return 3;
    }

    const int server_group = environment->get_server_group();
    if (server_group == 5 || server_group == 11) {
        return 24;
    }
    return 17;
}
