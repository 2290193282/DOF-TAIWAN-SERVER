#include "old_game/activity/schedule/advance_altar_data.h"

#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"

namespace advancealtar {

// IDA_EA: 0x813159A
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager16refillTicketFreeEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::refillTicketFree(void)
bool CharacAdvanceAltarManager::refillTicketFree() {
    bool result = false;
    const int refill_count =
        G_CDataManager()->get_advance_altar_config_parameter()
            ->daily_refill_ticket;

    for (int index = 0;; ++index) {
        result = index <= 0;
        if (index > 0) {
            break;
        }

        AdvanceAltarData& altar_data = data[index];
        if (isValidRidableId(
                static_cast<RidableId::T>(altar_data.ridable_id))) {
            altar_data.free_ticket_count = refill_count;
            altar_data.updated = 1;
        }
    }
    return result;
}

}  // namespace advancealtar
