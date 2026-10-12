#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_data/db/event/db_update_bingo_event.h"

#include <stdint.h>


// IDA_EA: 0x86914CC
// IDA_NAME: _ZN5CUser15updateBingoDataEv
// DEMANGLED: CUser::updateBingoData(void)
char* CUser::updateBingoData() {
    char* result = reinterpret_cast<char*>(
        static_cast<uintptr_t>(getBingoData()->reward_state));
    if (reinterpret_cast<uintptr_t>(result) != 0u) {
        const bool selected_count =
            static_cast<bool>(getBingoData()->selected_count);
        const int bingo_count =
            static_cast<int>(getBingoData()->bingo_count);
        DB_UpdateBingoEvent::makeRequest(this->GetUID(),
                                        static_cast<unsigned int>(
                                            this->get_acc_id()),
                                        bingo_count,
                                        selected_count);
        result = reinterpret_cast<char*>(getBingoData());
        result[6] = 0;
    }
    return result;
}
