#include "old_game/player/core/cuser.h"

#include <stdint.h>

// IDA_EA: 0x864F930
// IDA_NAME: _ZN5CUser20reset_tag_info_to_meEj
// DEMANGLED: CUser::reset_tag_info_to_me(unsigned int)
uint8_t CUser::reset_tag_info_to_me(uint32_t charac_no) {
    bool changed = false;

    for (std::vector<_Charac_info>::iterator it = characters_.begin();
         it != characters_.end();
         ++it) {
        _Charac_info::Fields& fields = it->fields_;
        if (fields.tag_character_no_ == charac_no) {
            fields.tag_character_no_ = 0;
            fields.striker_skill_index_ = 0;
            fields.tag_charac_info_loaded_ = 0;
            changed = true;
        }
    }

    if (changed) {
        return static_cast<uint8_t>(
            reinterpret_cast<uintptr_t>(character_view_.enableSaveCharacView()));
    }
    return 0;
}
