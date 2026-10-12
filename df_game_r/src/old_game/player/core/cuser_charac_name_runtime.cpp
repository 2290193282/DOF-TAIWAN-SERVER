#include "old_game/player/core/cuser.h"

#include <cstring>

// IDA_EA: 0x84ECC7C
// IDA_NAME: _ZN5CUser13setCharacNameEiPKc
// DEMANGLED: CUser::setCharacName(int,char const*)
int CUser::setCharacName(int charac_no, const char* charac_name) {
    for (std::vector<_Charac_info>::iterator it = characters_.begin();
         it != characters_.end();
         ++it) {
        if (it->fields_.character_no_ != charac_no) {
            continue;
        }

        // The old body is deliberately strncpy(…, 0x1d), rather than a
        // terminating copy: the persisted character-view field is 30 bytes.
        std::strncpy(it->fields_.character_name_, charac_name, 0x1du);
        character_view_.enableSaveCharacView();
        return 1;
    }
    return 0;
}
