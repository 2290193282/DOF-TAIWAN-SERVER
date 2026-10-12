#include "old_data/db/skill/item_making_skill_payload.h"

#include <cstring>


// IDA_EA: 0x848FA82
// IDA_NAME: _ZN21SIG_ITEM_MAKING_SKILLC2Ev
// DEMANGLED: SIG_ITEM_MAKING_SKILL::SIG_ITEM_MAKING_SKILL(void)
SIG_ITEM_MAKING_SKILL::SIG_ITEM_MAKING_SKILL()
    : charac_no(0) {
    std::memset(&weapon, 0, 0x16u);
}
