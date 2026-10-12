#include "old_game/economy/mall/npc_shop_value_lifecycle_types.h"

#include "old_data/PVF/npc/npc_script.h"

// IDA_EA: 0x89FB688
// IDA_NAME: _ZN11NpcDialogueC2Ev
// DEMANGLED: NpcDialogue::NpcDialogue(void)
NpcDialogue::NpcDialogue() {
    // The original constructs both strings and the vector first, then clears
    // all logical state.  Member construction is performed before this body.
    clear();
}

// IDA_EA: 0x89FB71C
// IDA_NAME: _ZN11NpcDialogue5clearEv
// DEMANGLED: NpcDialogue::clear(void)
void NpcDialogue::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    // Raw clear uses std::string::operator=("") rather than clear(), which
    // matters for the old COW-string ownership transition.
    text_0c = "";
    text_10 = "";
    values_14.clear();
}

// IDA_EA: 0x891CB4C
// IDA_NAME: _ZN22SkillShopGuideTextDataC2Ev
// DEMANGLED: SkillShopGuideTextData::SkillShopGuideTextData(void)
SkillShopGuideTextData::SkillShopGuideTextData() {
    // Raw C2 constructs only the string at +0x08.  The scalar range and RGB
    // bytes intentionally remain uninitialised until the RDAR parser fills
    // them, matching the old object lifecycle.
}

// IDA_EA: 0x891CB62
// IDA_NAME: _ZN22SkillShopGuideTextDataD2Ev
// DEMANGLED: SkillShopGuideTextData::~SkillShopGuideTextData()
SkillShopGuideTextData::~SkillShopGuideTextData() {}

// IDA_EA: 0x8932804
// IDA_NAME: _ZN22SkillShopGuideTextDataC2ERKS_
// DEMANGLED: SkillShopGuideTextData::SkillShopGuideTextData(SkillShopGuideTextData const&)
SkillShopGuideTextData::SkillShopGuideTextData(
    const SkillShopGuideTextData& rhs)
    : level_min(rhs.level_min),
      level_max(rhs.level_max),
      text(rhs.text),
      color_red(rhs.color_red),
      color_green(rhs.color_green),
      color_blue(rhs.color_blue) {}

// IDA_EA: 0x89328E6
// IDA_NAME: _ZN22SkillShopGuideTextDataaSERKS_
// DEMANGLED: SkillShopGuideTextData::operator=(SkillShopGuideTextData const&)
SkillShopGuideTextData& SkillShopGuideTextData::operator=(
    const SkillShopGuideTextData& rhs) {
    // The raw body performs every assignment even for self-assignment.
    level_min = rhs.level_min;
    level_max = rhs.level_max;
    text = rhs.text;
    color_red = rhs.color_red;
    color_green = rhs.color_green;
    color_blue = rhs.color_blue;
    return *this;
}

// IDA_EA: 0x8A06F28
// IDA_NAME: _ZN25stFavorableRelationShip_t19stRewardItemElement_tD2Ev
// DEMANGLED: stFavorableRelationShip_t::stRewardItemElement_t::~stRewardItemElement_t()
stFavorableRelationShip_t::stRewardItemElement_t::~stRewardItemElement_t() {}

// IDA_EA: 0x8A00D58
// IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_tC2Ev
// DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::stRewardItemInfo_t(void)
stFavorableRelationShip_t::stRewardItemInfo_t::stRewardItemInfo_t() {
    // The three vectors are constructed before the body; total_rates is left
    // untouched here exactly as in the raw C2.
}

// IDA_EA: 0x8581A80
// IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_tD2Ev
// DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::~stRewardItemInfo_t()
stFavorableRelationShip_t::stRewardItemInfo_t::~stRewardItemInfo_t() {}

// IDA_EA: 0x89FB4C0
// IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_t5clearEv
// DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::clear(void)
void stFavorableRelationShip_t::stRewardItemInfo_t::clear() {
    for (int favor = 0; favor <= 2; ++favor) {
        reward_items[favor].clear();
        total_rates[favor] = 0;
    }
}

// IDA_EA: 0x8A00DCC
// IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_tC2Ev
// DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::stRewardItem_t(void)
stFavorableRelationShip_t::stRewardItem_t::stRewardItem_t() {
    // Array member construction emits the original 5 x 3 Info C2 walk.
}

// IDA_EA: 0x8581AAE
// IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_tD2Ev
// DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::~stRewardItem_t()
stFavorableRelationShip_t::stRewardItem_t::~stRewardItem_t() {}

// IDA_EA: 0x89FB508
// IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_t5clearEv
// DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::clear(void)
void stFavorableRelationShip_t::stRewardItem_t::clear() {
    for (int mood = 0; mood <= 4; ++mood) {
        for (int favor = 0; favor <= 2; ++favor) {
            reward_items[mood][favor].clear();
        }
    }
}

// IDA_EA: 0x858188A
// IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoC2Ev
// DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo(void)
stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo()
{
    // The two strings are default-constructed before this body, then raw C2
    // writes +0x00, +0x04, and +0x08.  +0x02 is deliberately untouched.
    favor_level = 0;
    item_index = 0;
    item_count = 0;
}

// IDA_EA: 0x85823C6
// IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoD2Ev
// DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::~stLevelRewardInfo()
stFavorableRelationShip_t::stLevelRewardInfo::~stLevelRewardInfo() {}

// IDA_EA: 0x8A04160
// IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoC2ERKS0_
// DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo(stFavorableRelationShip_t::stLevelRewardInfo const&)
stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo(
    const stLevelRewardInfo& rhs)
    : favor_level(rhs.favor_level),
      item_index(rhs.item_index),
      item_count(rhs.item_count),
      title(rhs.title),
      text(rhs.text) {
    // The original copy C2 does not read or write reserved_02.
}

// IDA_EA: 0x8A04268
// IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoaSERKS0_
// DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::operator=(stFavorableRelationShip_t::stLevelRewardInfo const&)
stFavorableRelationShip_t::stLevelRewardInfo&
stFavorableRelationShip_t::stLevelRewardInfo::operator=(
    const stLevelRewardInfo& rhs) {
    // Like the copy C2, raw assignment leaves the reserved halfword intact.
    favor_level = rhs.favor_level;
    item_index = rhs.item_index;
    item_count = rhs.item_count;
    title = rhs.title;
    text = rhs.text;
    return *this;
}
