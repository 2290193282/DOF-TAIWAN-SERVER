#include "old_game/player/core/cuser.h"

#include <stdint.h>

// IDA_EA: 0x868F956
// IDA_NAME: _ZN5CUser22handleInformNoticeFlagEj
// DEMANGLED: CUser::handleInformNoticeFlag(unsigned int)
unsigned int CUser::handleInformNoticeFlag(unsigned int flag_index) {
    const unsigned int bit = flag_index & 7u;
    const unsigned int byte_index = flag_index >> 3;
    if (byte_index > 15u) {
        return bit;
    }

    const unsigned int can_set = checkInformNoticeFlag(bit, byte_index);
    if (!static_cast<unsigned char>(can_set)) {
        return can_set;
    }

    // checkInformNoticeFlag established that this bit is clear, so the
    // original addition at 0x868F9B6 is equivalent to this typed update.
    inform_notice_.flags[byte_index] = static_cast<uint8_t>(
        inform_notice_.flags[byte_index] + (1u << bit));
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(
        enableSaveInformNoticeFlag()));
}
