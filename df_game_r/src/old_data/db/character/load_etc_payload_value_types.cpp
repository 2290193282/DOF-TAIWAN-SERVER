#include "old_data/db/character/load_etc_payload.h"

#include <new>

// These leaf value constructors are kept in a separate translation unit from
// their owning arrays.  That preserves the old direct C2 loop topology instead
// of allowing GCC -O2 to fold their bodies into the container constructors.

// IDA_EA: 0x848F876
// IDA_NAME: _ZN11Redeem_Item28Inven_Item_Expand_RedeemInfoC2Ev
// DEMANGLED: Redeem_Item::Inven_Item_Expand_RedeemInfo::Inven_Item_Expand_RedeemInfo(void)
Redeem_Item::Inven_Item_Expand_RedeemInfo::Inven_Item_Expand_RedeemInfo()
    : expire_time(0x7FFFFFFF) {
    new (item_storage) Inven_Item();
}

// IDA_EA: 0x828D870
// IDA_NAME: _ZN13stAchievementC2Ev
// DEMANGLED: stAchievement::stAchievement(void)
stAchievement::stAchievement()
    : achievement_id(0), progress() {}

// IDA_EA: 0x832AEDE
// IDA_NAME: _ZN11MissionInfoC2Ev
// DEMANGLED: MissionInfo::MissionInfo(void)
MissionInfo::MissionInfo()
    : mission_kind(0),
      mission_index(0),
      current_value(0),
      target_value(0) {}

// IDA_EA: 0x85E68B6
// IDA_NAME: _ZN11MissionInfo5clearEv
// DEMANGLED: MissionInfo::clear(void)
MissionInfo* MissionInfo::clear() {
    mission_kind = 0;
    mission_index = 0;
    current_value = 0;
    target_value = 0;
    return this;
}
