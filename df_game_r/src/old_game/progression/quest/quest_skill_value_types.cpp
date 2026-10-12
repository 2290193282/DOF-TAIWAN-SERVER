#include "old_game/progression/quest/quest_skill_value_types.h"

#include "old_game/economy/equipment/item_option.h"

#include <map>
#include <new>
#include <string>


typedef char TagSkillPenalty_Size_Must_Be_0x10[
    (sizeof(TagSkillPenalty) == 0x10) ? 1 : -1];
typedef char STGrowthType_Size_Must_Be_0x01[
    (sizeof(STGrowthType) == 0x01) ? 1 : -1];
typedef char STGuildSkill_Size_Must_Be_0x05[
    (sizeof(STGuildSkill) == 0x05) ? 1 : -1];
typedef char comoboSkillData_Size_Must_Be_0x0e[
    (sizeof(comoboSkillData) == 0x0e) ? 1 : -1];
typedef char buySkillInfo_Size_Must_Be_0x0c[
    (sizeof(buySkillInfo) == 0x0c) ? 1 : -1];
typedef char LevelUpSkill_Size_Must_Be_0x0c[
    (sizeof(LevelUpSkill) == 0x0c) ? 1 : -1];
typedef char STSkillCommandData_STCommand_Size_Must_Be_0x04[
    (sizeof(STSkillCommandData::STCommand) == 0x04) ? 1 : -1];
typedef char STSkillCommandData_Size_Must_Be_0x0c[
    (sizeof(STSkillCommandData) == 0x0c) ? 1 : -1];
typedef char SKILL_USING_VALIDATE_INFO_Size_Must_Be_0x08[
    (sizeof(SKILL_USING_VALIDATE_INFO) == 0x08) ? 1 : -1];
typedef char SkillUseItem_Size_Must_Be_0x0c[
    (sizeof(SkillUseItem) == 0x0c) ? 1 : -1];
typedef char stQuestTriggerState_t_Size_Must_Be_0x08[
    (sizeof(stQuestTriggerState_t) == 0x08) ? 1 : -1];

// IDA_EA: 0x8486BCE
// IDA_NAME: _ZN12STGrowthTypeC2Ev
// DEMANGLED: STGrowthType::STGrowthType(void)
STGrowthType::STGrowthType()
    : growth_type(-1) {}

// STGuildSkill::STGuildSkill @ 0x8693F40 migrated to
// old_game/quest_skill/guild_skill_runtime.cpp.  Keep no second body here.

// IDA_EA: 0x848FB90
// IDA_NAME: _ZN15comoboSkillDataC2Ev
// DEMANGLED: comoboSkillData::comoboSkillData(void)
comoboSkillData::comoboSkillData()
    : skill_id(0),
      command() {}

// IDA_EA: 0x82309DE
// IDA_NAME: _ZN12buySkillInfoC2Ev
// DEMANGLED: buySkillInfo::buySkillInfo(void)
buySkillInfo::buySkillInfo()
    : job(0),
      tree(0),
      skill_id(-1),
      skill_level(-1) {}

// IDA_EA: 0x8180386
// IDA_NAME: _ZN12LevelUpSkill5clearEv
// DEMANGLED: LevelUpSkill::clear(void)
LevelUpSkill* LevelUpSkill::clear() {
    skill_type = 11;
    skill_id = 0;
    skill_level = 0;
    return this;
}

// IDA_EA: 0x81803A8
// IDA_NAME: _ZN12LevelUpSkillC2Ev
// DEMANGLED: LevelUpSkill::LevelUpSkill(void)
LevelUpSkill::LevelUpSkill() {
    clear();
}

// IDA_EA: 0x818250E
// IDA_NAME: _ZN12LevelUpSkillD2Ev
// DEMANGLED: LevelUpSkill::~LevelUpSkill()
LevelUpSkill::~LevelUpSkill() {}

// IDA_EA: 0x822DA04
// IDA_NAME: _ZN18STSkillCommandData9STCommandC2Ev
// DEMANGLED: STSkillCommandData::STCommand::STCommand(void)
STSkillCommandData::STCommand::STCommand() {
    raw[3] &= 0x1Fu;
    raw[3] = static_cast<uint8_t>(
        (4u * ((raw[3] >> 5) & 7u)) | (raw[3] & 0xE3u));
    const uint8_t v1 = static_cast<uint8_t>((raw[3] >> 2) & 7u);
    raw[2] = static_cast<uint8_t>(
        (((raw[3] & 4u) != 0u) << 7) | (raw[2] & 0x7Fu));
    raw[3] = static_cast<uint8_t>(((v1 >> 1) & 3u) | (raw[3] & 0xFCu));
    raw[2] = static_cast<uint8_t>(
        (16u * (((raw[2] >> 7) | (2u * (raw[3] & 3u))) & 7u)) |
        (raw[2] & 0x8Fu));
    raw[2] = static_cast<uint8_t>(
        (2u * ((raw[2] >> 4) & 7u)) | (raw[2] & 0xF1u));
    const uint8_t v2 = static_cast<uint8_t>((raw[2] >> 1) & 7u);
    raw[1] = static_cast<uint8_t>(((raw[2] >> 1) & 3u) << 6 |
                                  (raw[1] & 0x3Fu));
    raw[2] = static_cast<uint8_t>(((v2 & 4u) != 0u) | (raw[2] & 0xFEu));
    raw[0] = static_cast<uint8_t>((raw[1] >> 6) | (4u * (raw[2] & 1u)));
    raw[1] &= 0xC0u;
}

// IDA_EA: 0x822DB66
// IDA_NAME: _ZN18STSkillCommandDataC2Ev
// DEMANGLED: STSkillCommandData::STSkillCommandData(void)
STSkillCommandData::STSkillCommandData() {
    new (command_storage) STCommand();
    word_00 = 0;
    byte_02 = 0;
    for (int index = 0; index < 5; ++index) {
        raw_03[index] = 0;
    }
}

// IDA_EA: 0x8234756
// IDA_NAME: _ZN25SKILL_USING_VALIDATE_INFOC2Eihcc
// DEMANGLED: SKILL_USING_VALIDATE_INFO::SKILL_USING_VALIDATE_INFO(int,unsigned char,char,char)
SKILL_USING_VALIDATE_INFO::SKILL_USING_VALIDATE_INFO(
    int charac_job_value,
    uint8_t level_value,
    char first_grow_type_value,
    char second_grow_type_value)
    : charac_job(charac_job_value),
      level(level_value),
      first_grow_type(first_grow_type_value),
      second_grow_type(second_grow_type_value) {}

// IDA_EA: 0x8A9B118
// IDA_NAME: _ZN12SkillUseItem5clearEv
// DEMANGLED: SkillUseItem::clear(void)
bool SkillUseItem::clear() {
    item_index = -1;
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 1;
        if (index > 1) {
            break;
        }
        (&value_04)[index] = 0;
    }
    return result;
}

// IDA_EA: 0x8A9B150
// IDA_NAME: _ZN12SkillUseItemC2Ev
// DEMANGLED: SkillUseItem::SkillUseItem(void)
SkillUseItem::SkillUseItem() {
    clear();
}

// IDA_EA: 0x8A9C60E
// IDA_NAME: _ZN12SkillUseItemD2Ev
// DEMANGLED: SkillUseItem::~SkillUseItem()
SkillUseItem::~SkillUseItem() {}

// IDA_EA: 0x836DCE0
// IDA_NAME: _ZN21stQuestTriggerState_t10setTriggerEsss
// DEMANGLED: stQuestTriggerState_t::setTrigger(short,short,short)
stQuestTriggerState_t* stQuestTriggerState_t::setTrigger(
    int16_t trigger_0,
    int16_t trigger_1,
    int16_t trigger_2) {
    trigger_words[0] =
        static_cast<uint16_t>((trigger_0 & 0x1FF) | (trigger_words[0] & 0xFE00));
    trigger_words[1] =
        static_cast<uint16_t>((trigger_1 & 0x1FF) | (trigger_words[1] & 0xFE00));
    stQuestTriggerState_t* result = this;
    trigger_words[2] =
        static_cast<uint16_t>((trigger_2 & 0x1FF) | (trigger_words[2] & 0xFE00));
    return result;
}

// IDA_EA: 0x836DD68
// IDA_NAME: _ZN21stQuestTriggerState_t10getTriggerEv
// DEMANGLED: stQuestTriggerState_t::getTrigger(void)
int stQuestTriggerState_t::getTrigger() {
    return ((trigger_words[2] & 0x1FF) << 18) |
           ((trigger_words[1] & 0x1FF) << 9) |
           (trigger_words[0] & 0x1FF);
}

// IDA_EA: 0x836DDAC
// IDA_NAME: _ZN21stQuestTriggerState_tC2Ev
// DEMANGLED: stQuestTriggerState_t::stQuestTriggerState_t(void)
stQuestTriggerState_t::stQuestTriggerState_t()
    : trigger_words(),
      pad_06(0) {}

// IDA_EA: 0x86AD56C
// IDA_NAME: _ZN21stQuestTriggerState_taSERKi
// DEMANGLED: stQuestTriggerState_t::operator=(int const&)
int stQuestTriggerState_t::operator=(const int& packed_trigger) {
    trigger_words[0] = 0;
    trigger_words[1] = 0;
    trigger_words[2] = 0;
    pad_06 = 0;
    trigger_words[0] = static_cast<uint16_t>(
        (packed_trigger & 0x1FF) | (trigger_words[0] & 0xFE00));
    trigger_words[1] = static_cast<uint16_t>(
        (((packed_trigger & 0x3FE00) >> 9) & 0x1FF) |
        (trigger_words[1] & 0xFE00));
    int result = reinterpret_cast<int>(this);
    trigger_words[2] = static_cast<uint16_t>(
        (((packed_trigger & 0x7FC0000) >> 18) & 0x1FF) |
        (trigger_words[2] & 0xFE00));
    return result;
}

// IDA_EA: 0x86AD602
// IDA_NAME: _ZN21stQuestTriggerState_t15incTirggerStateEc
// DEMANGLED: stQuestTriggerState_t::incTirggerState(char)
uint32_t stQuestTriggerState_t::incTirggerState(char flags) {
    if ((flags & 0x10) != 0 && (trigger_words[0] & 0x1FFu) <= 0x3FE) {
        trigger_words[0] = static_cast<uint16_t>(
            (((trigger_words[0] & 0x1FF) + 1) & 0x1FF) |
            (trigger_words[0] & 0xFE00));
    }
    if ((flags & 0x20) != 0 && (trigger_words[1] & 0x1FFu) <= 0x3FE) {
        trigger_words[1] = static_cast<uint16_t>(
            (((trigger_words[1] & 0x1FF) + 1) & 0x1FF) |
            (trigger_words[1] & 0xFE00));
    }
    uint32_t result = flags & 0x40;
    if ((flags & 0x40) != 0) {
        result = trigger_words[2] & 0x1FF;
        if (result <= 0x3FE) {
            const uint32_t v4 =
                static_cast<uint32_t>((trigger_words[2] & 0x1FF) + 1) &
                0xFFFF01FFu;
            result = v4 | (trigger_words[2] & 0xFE00);
            trigger_words[2] = static_cast<uint16_t>(result);
        }
    }
    return result;
}

// IDA_EA: 0x86AD6EA
// IDA_NAME: _ZN21stQuestTriggerState_t15subTriggerStateEc
// DEMANGLED: stQuestTriggerState_t::subTriggerState(char)
stQuestTriggerState_t* stQuestTriggerState_t::subTriggerState(char flags) {
    stQuestTriggerState_t* result;
    if (flags) {
        if ((flags & 0x10) != 0 && (trigger_words[0] & 0x1FF) != 0) {
            trigger_words[0] = static_cast<uint16_t>(
                (((trigger_words[0] & 0x1FF) + 511) & 0x1FF) |
                (trigger_words[0] & 0xFE00));
        }
        if ((flags & 0x20) != 0 && (trigger_words[1] & 0x1FF) != 0) {
            trigger_words[1] = static_cast<uint16_t>(
                (((trigger_words[1] & 0x1FF) + 511) & 0x1FF) |
                (trigger_words[1] & 0xFE00));
        }
        result = reinterpret_cast<stQuestTriggerState_t*>(flags & 0x40);
        if ((flags & 0x40) != 0) {
            result =
                reinterpret_cast<stQuestTriggerState_t*>(trigger_words[2] & 0x1FF);
            if ((trigger_words[2] & 0x1FF) != 0) {
                const int v3 = trigger_words[2] & 0xFE00;
                result = reinterpret_cast<stQuestTriggerState_t*>(
                    (((trigger_words[2] & 0x1FF) + 511) & 0x1FF) | v3);
                trigger_words[2] = static_cast<uint16_t>(
                    reinterpret_cast<uintptr_t>(result));
            }
        }
    } else {
        result = this;
        // IDA 0x86AD6EA performs one little-endian dword decrement over the
        // first two physical trigger words when no selector flag is set.
        // Recompose that exact value through its named fields so a borrow
        // from trigger_words[0] into trigger_words[1] remains unchanged.
        uint32_t low_trigger_state =
            static_cast<uint32_t>(trigger_words[0]) |
            (static_cast<uint32_t>(trigger_words[1]) << 16u);
        --low_trigger_state;
        trigger_words[0] = static_cast<uint16_t>(low_trigger_state);
        trigger_words[1] =
            static_cast<uint16_t>(low_trigger_state >> 16u);
    }
    return result;
}

// IDA_EA: 0x80CB778
// IDA_NAME: _ZN22STEnchantedSkillOption5resetEv
// DEMANGLED: STEnchantedSkillOption::reset(void)
STEnchantedSkillOption* STEnchantedSkillOption::reset() {
    flags_ &= static_cast<uint8_t>(~1u);
    flags_ &= 0xF9u;
    STEnchantedSkillOption* result = this;
    flags_ &= 0x07u;
    return result;
}

namespace WongWork {

// IDA_EA: 0x848F768
// IDA_NAME: _ZN8WongWork14STClearedQuestC2Ev
// DEMANGLED: WongWork::STClearedQuest::STClearedQuest(void)
STClearedQuest::STClearedQuest()
    : clear_flag(0) {}

}  // namespace WongWork


// CSkill runtime migrated to old_game/quest_skill/cskill_runtime.cpp.
