#include "old_game/player/core/cuser.h"

#include <cstring>

// IDA_EA: 0x8693F56
// IDA_NAME: _ZN17STGuildDBInfoOnlyC2Ev
// DEMANGLED: STGuildDBInfoOnly::STGuildDBInfoOnly(void)
STGuildDBInfoOnly::STGuildDBInfoOnly() {
    // The original constructor invokes all sixteen five-byte STGuildSkill
    // constructors, then clears the packed DB payload before publication.
    // The array has already run those constructors on entry; reproduce the
    // final IDA state and the original guild-flag initialization exactly.
    std::memset(this, 0, sizeof(*this));
    guild_flag |= 1u;
    guild_flag &= static_cast<uint8_t>(~2u);
}
