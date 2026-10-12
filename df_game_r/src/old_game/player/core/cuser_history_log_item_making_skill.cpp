#include "old_game/player/core/cuser.h"


// IDA_EA: 0x8686A88
// IDA_NAME: _ZN15cUserHistoryLog18ItemMakingSkillAddE21ENUM_ITEM_MAKING_TYPEiis
// DEMANGLED: cUserHistoryLog::ItemMakingSkillAdd(ENUM_ITEM_MAKING_TYPE,int,int,short)
int cUserHistoryLog::ItemMakingSkillAdd(ENUM_ITEM_MAKING_TYPE type,
                                        int reason,
                                        int gain,
                                        short current) {
    return owner_->LogHistory(
        "ItemMakingSkill+,%d,%d,%d,%d", type, reason, gain, current);
}
