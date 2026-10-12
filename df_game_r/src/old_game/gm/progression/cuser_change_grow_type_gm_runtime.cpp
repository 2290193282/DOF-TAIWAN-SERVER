#include "old_game/player/core/cuser.h"

#include "old_game/progression/quest/force_change_grow_type_quest_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/premium_runtime.h"

#include <iostream>

// IDA_EA: 0x867B048
// IDA_NAME: _ZN5CUser17ChangeGrowType_GMEii
// DEMANGLED: CUser::ChangeGrowType_GM(int,int)
int CUser::ChangeGrowType_GM(int first_grow_type, int second_grow_type) {
    if (!getCurCharacR()) {
        return 0;
    }

    if (first_grow_type < 0 || first_grow_type > 5 ||
        second_grow_type < 0 || second_grow_type > 1) {
        std::cerr << "Invalid grow type , first :" << first_grow_type
                  << "second :" << second_grow_type << std::endl;
        return 0;
    }

    const int character_level =
        get_charac_level() + GetPremiumInfo()->getOverSkillLevel();
    getCurCharacSkillW()->debugCheckGrowTypeSkill(character_level,
                                                   get_charac_job(),
                                                   first_grow_type,
                                                   second_grow_type);
    set_grow_type(static_cast<unsigned char>(first_grow_type),
                  static_cast<unsigned char>(second_grow_type),
                  0,
                  static_cast<eChangeGrowTypeReason>(2));
    _postCheckForceChangeGrowType(this, first_grow_type, second_grow_type);
    return 1;
}
