#include "old_game/player/core/arad_character_signals.h"

#include <cstring>

namespace arad {

typedef char SigAradJumpingCharacter_Size_Must_Be_0x09[
    (sizeof(SigAradJumpingCharacter) == 0x09) ? 1 : -1];

// IDA_EA: 0x81A0990
// IDA_NAME: _ZN4arad23SigAradJumpingCharacterC2Ev
// DEMANGLED: arad::SigAradJumpingCharacter::SigAradJumpingCharacter(void)
SigAradJumpingCharacter::SigAradJumpingCharacter() {
    init();
}

// IDA_EA: 0x81A09A4
// IDA_NAME: _ZN4arad23SigAradJumpingCharacter4initEv
// DEMANGLED: arad::SigAradJumpingCharacter::init(void)
SigAradJumpingCharacter* SigAradJumpingCharacter::init() {
    std::memset(this, 0, 9);
    return this;
}

}  // namespace arad
