#include "old_game/player/session/character_select_params.h"

#include <new>


typedef char stSelectCharac_Size_Must_Be_0x08[
    (sizeof(stSelectCharac) == 0x08) ? 1 : -1];
typedef char stCreateCharac_Size_Must_Be_0x08[
    (sizeof(stCreateCharac) == 0x08) ? 1 : -1];
typedef char stDeleteCharac_Size_Must_Be_0x08[
    (sizeof(stDeleteCharac) == 0x08) ? 1 : -1];
typedef char stDieCharacInDungeon_Size_Must_Be_0x08[
    (sizeof(stDieCharacInDungeon) == 0x08) ? 1 : -1];
typedef char stSaveCharacOption_Size_Must_Be_0x08[
    (sizeof(stSaveCharacOption) == 0x08) ? 1 : -1];
typedef char stRequestCharacSkillInfo_Size_Must_Be_0x08[
    (sizeof(stRequestCharacSkillInfo) == 0x08) ? 1 : -1];
typedef char stComboSkillExtensionQuickSlotReset_Size_Must_Be_0x08[
    (sizeof(stComboSkillExtensionQuickSlotReset) == 0x08) ? 1 : -1];
typedef char stEquipslotSwitch_Size_Must_Be_0x08[
    (sizeof(stEquipslotSwitch) == 0x08) ? 1 : -1];
typedef char stExpandEquipslotFlagUpdate_Size_Must_Be_0x08[
    (sizeof(stExpandEquipslotFlagUpdate) == 0x08) ? 1 : -1];
typedef char stCharacSlotExtendEffect_Size_Must_Be_0x0c[
    (sizeof(stCharacSlotExtendEffect) == 0x0c) ? 1 : -1];

// IDA_EA: 0x8230DB2
// IDA_NAME: _ZN14stSelectCharacC2Ev
// DEMANGLED: stSelectCharac::stSelectCharac(void)
stSelectCharac::stSelectCharac()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8247F2A
// IDA_NAME: _ZN14stSelectCharacD2Ev
// DEMANGLED: stSelectCharac::~stSelectCharac()
stSelectCharac::~stSelectCharac() {}

// IDA_EA: 0x8247F5A
// IDA_NAME: _ZN14stSelectCharacD0Ev
// DEMANGLED: stSelectCharac::~stSelectCharac() deleting destructor
void stSelectCharac_deleting_destructor(stSelectCharac* self) {
    if (self) {
        self->~stSelectCharac();
        operator delete(self);
    }
}

// IDA_EA: 0x8230ABE
// IDA_NAME: _ZN14stCreateCharacC2Ev
// DEMANGLED: stCreateCharac::stCreateCharac(void)
stCreateCharac::stCreateCharac()
    : ParamBase(), result(0) {}

// IDA_EA: 0x82483BC
// IDA_NAME: _ZN14stCreateCharacD2Ev
// DEMANGLED: stCreateCharac::~stCreateCharac()
stCreateCharac::~stCreateCharac() {}

// IDA_EA: 0x82483EC
// IDA_NAME: _ZN14stCreateCharacD0Ev
// DEMANGLED: stCreateCharac::~stCreateCharac() deleting destructor
void stCreateCharac_deleting_destructor(stCreateCharac* self) {
    if (self) {
        self->~stCreateCharac();
        operator delete(self);
    }
}

// IDA_EA: 0x8230B12
// IDA_NAME: _ZN14stDeleteCharacC2Ev
// DEMANGLED: stDeleteCharac::stDeleteCharac(void)
stDeleteCharac::stDeleteCharac()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8248320
// IDA_NAME: _ZN14stDeleteCharacD2Ev
// DEMANGLED: stDeleteCharac::~stDeleteCharac()
stDeleteCharac::~stDeleteCharac() {}

// IDA_EA: 0x8248350
// IDA_NAME: _ZN14stDeleteCharacD0Ev
// DEMANGLED: stDeleteCharac::~stDeleteCharac() deleting destructor
void stDeleteCharac_deleting_destructor(stDeleteCharac* self) {
    if (self) {
        self->~stDeleteCharac();
        operator delete(self);
    }
}

// IDA_EA: 0x8230BB2
// IDA_NAME: _ZN20stDieCharacInDungeonC2Ev
// DEMANGLED: stDieCharacInDungeon::stDieCharacInDungeon(void)
stDieCharacInDungeon::stDieCharacInDungeon()
    : ParamBase(), dword_04(0) {}

// IDA_EA: 0x8248284
// IDA_NAME: _ZN20stDieCharacInDungeonD2Ev
// DEMANGLED: stDieCharacInDungeon::~stDieCharacInDungeon()
stDieCharacInDungeon::~stDieCharacInDungeon() {}

// IDA_EA: 0x82482B4
// IDA_NAME: _ZN20stDieCharacInDungeonD0Ev
// DEMANGLED: stDieCharacInDungeon::~stDieCharacInDungeon() deleting destructor
void stDieCharacInDungeon_deleting_destructor(stDieCharacInDungeon* self) {
    if (self) {
        self->~stDieCharacInDungeon();
        operator delete(self);
    }
}

// IDA_EA: 0x823123C
// IDA_NAME: _ZN18stSaveCharacOptionC2Ev
// DEMANGLED: stSaveCharacOption::stSaveCharacOption(void)
stSaveCharacOption::stSaveCharacOption()
    : ParamBase(), dword_04(0) {}

// IDA_EA: 0x82476F0
// IDA_NAME: _ZN18stSaveCharacOptionD2Ev
// DEMANGLED: stSaveCharacOption::~stSaveCharacOption()
stSaveCharacOption::~stSaveCharacOption() {}

// IDA_EA: 0x8247720
// IDA_NAME: _ZN18stSaveCharacOptionD0Ev
// DEMANGLED: stSaveCharacOption::~stSaveCharacOption() deleting destructor
void stSaveCharacOption_deleting_destructor(stSaveCharacOption* self) {
    if (self) {
        self->~stSaveCharacOption();
        operator delete(self);
    }
}

// IDA_EA: 0x824A63C
// IDA_NAME: _ZN24stRequestCharacSkillInfoC2Ev
// DEMANGLED: stRequestCharacSkillInfo::stRequestCharacSkillInfo(void)
stRequestCharacSkillInfo::stRequestCharacSkillInfo()
    : ParamBase() {}

// IDA_EA: 0x824A658
// IDA_NAME: _ZN24stRequestCharacSkillInfoD2Ev
// DEMANGLED: stRequestCharacSkillInfo::~stRequestCharacSkillInfo()
stRequestCharacSkillInfo::~stRequestCharacSkillInfo() {}

// IDA_EA: 0x824A688
// IDA_NAME: _ZN24stRequestCharacSkillInfoD0Ev
// DEMANGLED: stRequestCharacSkillInfo::~stRequestCharacSkillInfo() deleting destructor
void stRequestCharacSkillInfo_deleting_destructor(
    stRequestCharacSkillInfo* self) {
    if (self) {
        self->~stRequestCharacSkillInfo();
        operator delete(self);
    }
}

// IDA_EA: 0x8260520
// IDA_NAME: _ZN35stComboSkillExtensionQuickSlotResetC2Ev
// DEMANGLED: stComboSkillExtensionQuickSlotReset::stComboSkillExtensionQuickSlotReset(void)
stComboSkillExtensionQuickSlotReset::stComboSkillExtensionQuickSlotReset()
    : ParamBase(), dword_04(0) {}

// IDA_EA: 0x8260CB0
// IDA_NAME: _ZN35stComboSkillExtensionQuickSlotResetD2Ev
// DEMANGLED: stComboSkillExtensionQuickSlotReset::~stComboSkillExtensionQuickSlotReset()
stComboSkillExtensionQuickSlotReset::~stComboSkillExtensionQuickSlotReset() {}

// IDA_EA: 0x8260CE0
// IDA_NAME: _ZN35stComboSkillExtensionQuickSlotResetD0Ev
// DEMANGLED: stComboSkillExtensionQuickSlotReset::~stComboSkillExtensionQuickSlotReset() deleting destructor
void stComboSkillExtensionQuickSlotReset_deleting_destructor(
    stComboSkillExtensionQuickSlotReset* self) {
    if (self) {
        self->~stComboSkillExtensionQuickSlotReset();
        operator delete(self);
    }
}

// IDA_EA: 0x823162C
// IDA_NAME: _ZN17stEquipslotSwitchC2Ev
// DEMANGLED: stEquipslotSwitch::stEquipslotSwitch(void)
stEquipslotSwitch::stEquipslotSwitch()
    : ParamBase(), byte_04(0) {}

// IDA_EA: 0x8246F52
// IDA_NAME: _ZN17stEquipslotSwitchD2Ev
// DEMANGLED: stEquipslotSwitch::~stEquipslotSwitch()
stEquipslotSwitch::~stEquipslotSwitch() {}

// IDA_EA: 0x8246F82
// IDA_NAME: _ZN17stEquipslotSwitchD0Ev
// DEMANGLED: stEquipslotSwitch::~stEquipslotSwitch() deleting destructor
void stEquipslotSwitch_deleting_destructor(stEquipslotSwitch* self) {
    if (self) {
        self->~stEquipslotSwitch();
        operator delete(self);
    }
}

// IDA_EA: 0x8231650
// IDA_NAME: _ZN27stExpandEquipslotFlagUpdateC2Ev
// DEMANGLED: stExpandEquipslotFlagUpdate::stExpandEquipslotFlagUpdate(void)
stExpandEquipslotFlagUpdate::stExpandEquipslotFlagUpdate()
    : ParamBase(), byte_04(0) {}

// IDA_EA: 0x8246F04
// IDA_NAME: _ZN27stExpandEquipslotFlagUpdateD2Ev
// DEMANGLED: stExpandEquipslotFlagUpdate::~stExpandEquipslotFlagUpdate()
stExpandEquipslotFlagUpdate::~stExpandEquipslotFlagUpdate() {}

// IDA_EA: 0x8246F34
// IDA_NAME: _ZN27stExpandEquipslotFlagUpdateD0Ev
// DEMANGLED: stExpandEquipslotFlagUpdate::~stExpandEquipslotFlagUpdate() deleting destructor
void stExpandEquipslotFlagUpdate_deleting_destructor(
    stExpandEquipslotFlagUpdate* self) {
    if (self) {
        self->~stExpandEquipslotFlagUpdate();
        operator delete(self);
    }
}

// IDA_EA: 0x8231726
// IDA_NAME: _ZN24stCharacSlotExtendEffectC2Ev
// DEMANGLED: stCharacSlotExtendEffect::stCharacSlotExtendEffect(void)
stCharacSlotExtendEffect::stCharacSlotExtendEffect()
    : ParamBase(),
      result_code(0),
      slot_effect_count(0),
      charac_slot_limit(0) {}

// IDA_EA: 0x8246D7E
// IDA_NAME: _ZN24stCharacSlotExtendEffectD2Ev
// DEMANGLED: stCharacSlotExtendEffect::~stCharacSlotExtendEffect()
stCharacSlotExtendEffect::~stCharacSlotExtendEffect() {}

// IDA_EA: 0x8246DAE
// IDA_NAME: _ZN24stCharacSlotExtendEffectD0Ev
// DEMANGLED: stCharacSlotExtendEffect::~stCharacSlotExtendEffect() deleting destructor
void stCharacSlotExtendEffect_deleting_destructor(
    stCharacSlotExtendEffect* self) {
    if (self) {
        self->~stCharacSlotExtendEffect();
        operator delete(self);
    }
}
