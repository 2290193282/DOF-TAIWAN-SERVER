#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_SELECT_PARAMS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_SELECT_PARAMS_H_

#include <stdint.h>

#include "infra/network/packet/message_base.h"


// IDA unit: 04_character_select_create/stSelectCharac.
struct stSelectCharac : public ParamBase {
    stSelectCharac();
    virtual ~stSelectCharac();

    int32_t result;  // IDA +0x04
};

void stSelectCharac_deleting_destructor(stSelectCharac* self);

// IDA unit: 04_character_select_create/stCreateCharac.
struct stCreateCharac : public ParamBase {
    stCreateCharac();
    virtual ~stCreateCharac();

    int32_t result;  // IDA +0x04
};

void stCreateCharac_deleting_destructor(stCreateCharac* self);

// IDA unit: 04_character_select_create/stDeleteCharac.
struct stDeleteCharac : public ParamBase {
    stDeleteCharac();
    virtual ~stDeleteCharac();

    int32_t result;  // IDA +0x04
};

void stDeleteCharac_deleting_destructor(stDeleteCharac* self);

// IDA unit: 04_character_select_create/stDieCharacInDungeon.
struct stDieCharacInDungeon : public ParamBase {
    stDieCharacInDungeon();
    virtual ~stDieCharacInDungeon();

    uint32_t dword_04;  // IDA +0x04
};

void stDieCharacInDungeon_deleting_destructor(stDieCharacInDungeon* self);

// IDA unit: 04_character_select_create/stSaveCharacOption.
struct stSaveCharacOption : public ParamBase {
    stSaveCharacOption();
    virtual ~stSaveCharacOption();

    int32_t dword_04;  // IDA +0x04
};

void stSaveCharacOption_deleting_destructor(stSaveCharacOption* self);

// IDA unit: 04_character_select_create/stRequestCharacSkillInfo.
struct stRequestCharacSkillInfo : public ParamBase {
    stRequestCharacSkillInfo();
    virtual ~stRequestCharacSkillInfo();

    int32_t dword_04;  // IDA +0x04; raw C2 intentionally leaves it unset
};

void stRequestCharacSkillInfo_deleting_destructor(
    stRequestCharacSkillInfo* self);

// IDA unit: 04_character_select_create/stComboSkillExtensionQuickSlotReset.
struct stComboSkillExtensionQuickSlotReset : public ParamBase {
    stComboSkillExtensionQuickSlotReset();
    virtual ~stComboSkillExtensionQuickSlotReset();

    int32_t dword_04;  // IDA +0x04
};

void stComboSkillExtensionQuickSlotReset_deleting_destructor(
    stComboSkillExtensionQuickSlotReset* self);

// IDA unit: 04_character_select_create/stEquipslotSwitch.
struct stEquipslotSwitch : public ParamBase {
    stEquipslotSwitch();
    virtual ~stEquipslotSwitch();

    uint8_t byte_04;  // IDA +0x04
};

void stEquipslotSwitch_deleting_destructor(stEquipslotSwitch* self);

// IDA unit: 04_character_select_create/stExpandEquipslotFlagUpdate.
struct stExpandEquipslotFlagUpdate : public ParamBase {
    stExpandEquipslotFlagUpdate();
    virtual ~stExpandEquipslotFlagUpdate();

    uint8_t byte_04;  // IDA +0x04
};

void stExpandEquipslotFlagUpdate_deleting_destructor(
    stExpandEquipslotFlagUpdate* self);

// IDA unit: 04_character_select_create/stCharacSlotExtendEffect.
struct stCharacSlotExtendEffect : public ParamBase {
    stCharacSlotExtendEffect();
    virtual ~stCharacSlotExtendEffect();

    int32_t result_code;             // IDA +0x04
    uint8_t slot_effect_count;       // IDA +0x08
    uint8_t charac_slot_limit;       // IDA +0x09
};

void stCharacSlotExtendEffect_deleting_destructor(
    stCharacSlotExtendEffect* self);

#if defined(__i386__)
static_assert(sizeof(stSelectCharac) == 0x08u &&
                  offsetof(stSelectCharac, result) == 0x04u,
              "stSelectCharac result ABI changed");
static_assert(sizeof(stCreateCharac) == 0x08u &&
                  offsetof(stCreateCharac, result) == 0x04u,
              "stCreateCharac result ABI changed");
static_assert(sizeof(stDeleteCharac) == 0x08u &&
                  offsetof(stDeleteCharac, result) == 0x04u,
              "stDeleteCharac result ABI changed");
static_assert(sizeof(stCharacSlotExtendEffect) == 0x0cu &&
                  offsetof(stCharacSlotExtendEffect, result_code) == 0x04u &&
                  offsetof(stCharacSlotExtendEffect, slot_effect_count) ==
                      0x08u &&
                  offsetof(stCharacSlotExtendEffect, charac_slot_limit) ==
                      0x09u,
              "stCharacSlotExtendEffect named result ABI changed");
#endif


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_SELECT_PARAMS_H_
