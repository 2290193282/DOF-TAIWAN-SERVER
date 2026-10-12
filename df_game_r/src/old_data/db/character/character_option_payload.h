#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_OPTION_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_OPTION_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/player/setting/character_option.h"


// Canonical signal-693 payload.  Keeping the named type complete at the
// Stream specialization removes the opaque 516-byte allocation without
// changing the proven 4-byte identity + 0x200 option wire layout.
#pragma pack(push, 1)
struct SIG_CHARACTER_OPTION {
    uint32_t charac_no;
    CharacterOption option;
};
#pragma pack(pop)

static_assert(sizeof(SIG_CHARACTER_OPTION) == 0x204u &&
                  offsetof(SIG_CHARACTER_OPTION, option) == 0x04u,
              "character-option DB payload ABI changed");

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_OPTION_PAYLOAD_H_
