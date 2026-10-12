#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868AA54
// IDA_NAME: _ZN5CUser19UpdateEquipslotFlagEc
// DEMANGLED: CUser::UpdateEquipslotFlag(char)
int CUser::UpdateEquipslotFlag(char flag) {
    if (get_charac_level() <= 44) {
        return 0;
    }

    if (flag != 0) {
        if (flag == 2) {
            setCurEquipslotFlag(2);
            return getCurEquipslotFlag();
        }
        return 19;
    }

    if (static_cast<unsigned char>(getCurEquipslotFlag()) == 2u) {
        return 2;
    }

    setCurEquipslotFlag(1);
    return getCurEquipslotFlag();
}
