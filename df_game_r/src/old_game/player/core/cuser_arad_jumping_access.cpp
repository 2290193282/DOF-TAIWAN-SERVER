#include "old_game/player/core/cuser.h"

// Non-IDA C-owner bridge.  The three callees are the original old-server
// CUser character-list surface:
//   get_charac_count       @ 0x82300E8
//   get_charac_no(int)     @ 0x864DFA0
//   getCharacLevelByCharacNo(int) const @ 0x84ECBE0
// CMD585's original check iterates this same list and accepts when any
// character has a level strictly above its supplied limit.
bool CUser::HasCharacterLevelAbove(int level) const {
    CUser* const writable_user = const_cast<CUser*>(this);
    const int character_count = writable_user->get_charac_count();
    for (int slot = 0; slot < character_count; ++slot) {
        const int charac_no = writable_user->get_charac_no(slot);
        if (getCharacLevelByCharacNo(charac_no) > level) {
            return true;
        }
    }
    return false;
}
