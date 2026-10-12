#include "old_game/activity/schedule/inter_advance_altar_load.h"

#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/player/core/cuser.h"

namespace advancealtar {

// IDA_EA: 0x813F72A
// IDA_NAME: _ZN12advancealtar10Inter_Load12dispatch_sigEP5CUserPci
// DEMANGLED: advancealtar::Inter_Load::dispatch_sig(CUser *,char *,int)
int Inter_Load::dispatch_sig(CUser* user, char* payload, int /*payload_size*/) {
    if (!user || !user->get_state() || !user->getCurCharacR()) {
        return 734;
    }
    if (!payload) {
        return 739;
    }

    // The database wire record is exactly _CharacAdvanceAltarDbData.  The
    // original receiver passes it to the in-object manager without a copy or
    // intermediate byte facade.
    user->advance_altar_manager_.set(
        *reinterpret_cast<const _CharacAdvanceAltarDbData*>(payload));
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(10));
    return 0;
}

// IDA_EA: 0x84ED464 / 0x814213C / 0x814216C
// IDA_NAME: _ZN12advancealtar10Inter_LoadC2Ev /
//           _ZN12advancealtar10Inter_LoadD2Ev /
//           _ZN12advancealtar10Inter_LoadD0Ev
Inter_Load::Inter_Load() : ISigDispatcher() {}
Inter_Load::~Inter_Load() {}

}  // namespace advancealtar
