#include "old_game/player/core/cuser.h"

#include <cstring>
#include <stdint.h>

#include "old_data/db/character/load_etc_payload.h"


// IDA_EA: 0x868148C
// IDA_NAME: _ZN5CUser19SaveBackupSkillInfoEP22SIG_LOAD_PREMIUM_LETHE20ENUM_SKILL_TREE_KIND
// DEMANGLED: CUser::SaveBackupSkillInfo(SIG_LOAD_PREMIUM_LETHE *,ENUM_SKILL_TREE_KIND)
uint8_t CUser::SaveBackupSkillInfo(SIG_LOAD_PREMIUM_LETHE* source,
                                   ENUM_SKILL_TREE_KIND skill_tree) {
    uint8_t result = getCurCharacR() == 0;
    if (!result) {
        if (skill_tree != ENUM_SKILL_TREE_CURRENT && skill_tree) {
            result = GetCurCharacBackupSkill2ND() != 0;
            if (result) {
                SkillSlot* const destination = GetCurCharacBackupSkill2ND();
                return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(
                    std::memcpy(destination, source, 0x199u)));
            }
        } else {
            result = GetCurCharacBackupSkill() != 0;
            if (result) {
                SkillSlot* const destination = GetCurCharacBackupSkill();
                return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(
                    std::memcpy(destination, source, 0x199u)));
            }
        }
    }
    return result;
}
