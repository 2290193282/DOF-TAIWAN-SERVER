#ifndef OLDSRV_OLD_GAME_NPC_NPC_SHOP_VALUE_LIFECYCLE_TYPES_H_
#define OLDSRV_OLD_GAME_NPC_NPC_SHOP_VALUE_LIFECYCLE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

// IDA unit: 07_npc_shop_dialog/NpcDialogue.
//
// The old i386 object is 0x20 bytes: three scalar selectors, two COW-string
// handles, and a vector<int>.  The field roles are not named by the original
// export, so retain neutral offset-based names rather than invent semantics.
class NpcDialogue {
public:
    // IDA_EA: 0x89FB688
    // IDA_NAME: _ZN11NpcDialogueC2Ev
    // DEMANGLED: NpcDialogue::NpcDialogue(void)
    NpcDialogue();

    // IDA_EA: 0x89FB71C
    // IDA_NAME: _ZN11NpcDialogue5clearEv
    // DEMANGLED: NpcDialogue::clear(void)
    void clear();

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    std::string text_0c;
    std::string text_10;
    std::vector<int> values_14;
};

// IDA unit: 07_npc_shop_dialog/SkillShopGuideTextData.
// `importSkillShopGuideText @ 0x8914483` proves the two leading words are the
// [level range], the string is [text], and the trailing bytes are [color].
struct SkillShopGuideTextData {
    // IDA_EA: 0x891CB4C
    // IDA_NAME: _ZN22SkillShopGuideTextDataC2Ev
    // DEMANGLED: SkillShopGuideTextData::SkillShopGuideTextData(void)
    SkillShopGuideTextData();

    // IDA_EA: 0x891CB62
    // IDA_NAME: _ZN22SkillShopGuideTextDataD2Ev
    // DEMANGLED: SkillShopGuideTextData::~SkillShopGuideTextData()
    ~SkillShopGuideTextData();

    // IDA_EA: 0x8932804
    // IDA_NAME: _ZN22SkillShopGuideTextDataC2ERKS_
    // DEMANGLED: SkillShopGuideTextData::SkillShopGuideTextData(SkillShopGuideTextData const&)
    SkillShopGuideTextData(const SkillShopGuideTextData& rhs);

    // IDA_EA: 0x89328E6
    // IDA_NAME: _ZN22SkillShopGuideTextDataaSERKS_
    // DEMANGLED: SkillShopGuideTextData::operator=(SkillShopGuideTextData const&)
    SkillShopGuideTextData& operator=(const SkillShopGuideTextData& rhs);

    int32_t level_min;
    int32_t level_max;
    std::string text;
    uint8_t color_red;
    uint8_t color_green;
    uint8_t color_blue;
};

// These guards intentionally bind the providers to the original CentOS6 i386
// libstdc++ COW layout.  A modern C++11 string ABI must not silently masquerade
// as the old server's packet/PVF value layout.
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(std::string) == 0x04u,
              "old df_game_r requires the i386 COW std::string ABI");
static_assert(sizeof(std::vector<int>) == 0x0Cu,
              "old df_game_r requires the i386 std::vector ABI");
static_assert(sizeof(NpcDialogue) == 0x20u,
              "NpcDialogue IDA ABI size must stay 0x20");
static_assert(offsetof(NpcDialogue, text_0c) == 0x0Cu,
              "NpcDialogue text_0c must stay at +0x0c");
static_assert(offsetof(NpcDialogue, text_10) == 0x10u,
              "NpcDialogue text_10 must stay at +0x10");
static_assert(offsetof(NpcDialogue, values_14) == 0x14u,
              "NpcDialogue values_14 must stay at +0x14");
static_assert(sizeof(SkillShopGuideTextData) == 0x10u,
              "SkillShopGuideTextData IDA ABI size must stay 0x10");
static_assert(offsetof(SkillShopGuideTextData, text) == 0x08u,
              "SkillShopGuideTextData text must stay at +0x08");
static_assert(offsetof(SkillShopGuideTextData, color_red) == 0x0Cu,
              "SkillShopGuideTextData RGB must start at +0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_NPC_SHOP_VALUE_LIFECYCLE_TYPES_H_
