#include "old_game/player/core/cuser.h"
#include "old_data/db/event/db_update_blue_marble_enter_count.h"

// IDA_EA: 0x869106C
// IDA_NAME: _ZN5CUser26updateBlueMarbleEnterCountEv
// DEMANGLED: CUser::updateBlueMarbleEnterCount(void)
void CUser::updateBlueMarbleEnterCount() {
    const unsigned char enter_count =
        static_cast<unsigned char>(getBlueMarbleEnterCount());
    const unsigned int charac_no =
        static_cast<unsigned int>(get_charac_no(-1));
    DB_UpdateBlueMarbleEnterCount::makeRequest(charac_no, enter_count);
}
