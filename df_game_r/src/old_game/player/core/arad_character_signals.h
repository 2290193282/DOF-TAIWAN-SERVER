#ifndef OLDSRV_CHARACTER_SELECT_ARAD_CHARACTER_SIGNALS_H_
#define OLDSRV_CHARACTER_SELECT_ARAD_CHARACTER_SIGNALS_H_

#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"

#include <stdint.h>

namespace arad {

// IDA unit: 04_character_select_create/ARAD/SigAradJumpingCharacter.
struct SigAradJumpingCharacter {
    SigAradJumpingCharacter();

    SigAradJumpingCharacter* init();

    uint8_t raw[9];
};

}  // namespace arad

#endif  // OLDSRV_CHARACTER_SELECT_ARAD_CHARACTER_SIGNALS_H_
