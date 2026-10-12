#include "old_game/player/core/cuser.h"

// IDA_EA: 0x86887C4
// IDA_NAME: _ZN5CUser20isDuplicationMessageERKSs
// DEMANGLED: CUser::isDuplicationMessage(std::string const&)
bool CUser::isDuplicationMessage(const std::string& message) {
    const bool duplicated =
        user_strings_[0] == message || user_strings_[1] == message;
    user_strings_[duplicate_message_index_] = message;
    duplicate_message_index_ ^= 1u;
    return duplicated;
}
