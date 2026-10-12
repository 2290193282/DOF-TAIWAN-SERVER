#include "old_game/player/core/cuser.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/db/account/login_payloads.h"
#include "old_game/activity/schedule/event_runtime.h"

// IDA_EA: 0x868D664
// IDA_NAME: _ZN5CUser24set_ingame_event_historyEP24SIG_INGAME_EVENT_HISTORY
// DEMANGLED: CUser::set_ingame_event_history(SIG_INGAME_EVENT_HISTORY *)
//
// Inter_LoginInfo passes the event-history view at original SIG_LOGIN_DATA
// +0x3FAC. The expanded character view moves it to +0x40CC. The old code does
// not import every database record: it first asks the live event-script
// manager whether that record type is valid in the currently loaded event set,
// then uses the existing per-record state transition.
void CUser::set_ingame_event_history(SIG_INGAME_EVENT_HISTORY* history) {
    if (!history) {
        return;
    }

    for (int index = 0; index <= 29; ++index) {
        const SIG_INGAME_EVENT_HISTORY::Entry& entry = history->entries[index];
        if (entry.type == 0) {
            continue;
        }

        if (G_CDataManager()->get_event_script_mng()
                ->is_current_ingame_event_record_type(entry.type)) {
            set_ingame_event_history(entry.type, entry.first, entry.second);
        }
    }
}
