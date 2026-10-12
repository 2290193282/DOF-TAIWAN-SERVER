#include "old_game/player/core/creature_guild_params.h"

#include <new>


typedef char stGrowthEquipment_Size_Must_Be_0x18[
    (sizeof(stGrowthEquipment) == 0x18) ? 1 : -1];
typedef char stGrowthCreature_Size_Must_Be_0x08[
    (sizeof(stGrowthCreature) == 0x08) ? 1 : -1];
typedef char stHatchCreature_Size_Must_Be_0x08[
    (sizeof(stHatchCreature) == 0x08) ? 1 : -1];
typedef char stRenameCreature_Size_Must_Be_0x08[
    (sizeof(stRenameCreature) == 0x08) ? 1 : -1];
typedef char stGuildCargoParam_Size_Must_Be_0x08[
    (sizeof(stGuildCargoParam) == 0x08) ? 1 : -1];
typedef char stGuildCargoPushItemParam_Size_Must_Be_0x08[
    (sizeof(stGuildCargoPushItemParam) == 0x08) ? 1 : -1];
typedef char stGuildCargoPopItemParam_Size_Must_Be_0x08[
    (sizeof(stGuildCargoPopItemParam) == 0x08) ? 1 : -1];
typedef char stGuildCargoMoveItemParam_Size_Must_Be_0x08[
    (sizeof(stGuildCargoMoveItemParam) == 0x08) ? 1 : -1];
typedef char stGuildCargoHistoryParam_Size_Must_Be_0x08[
    (sizeof(stGuildCargoHistoryParam) == 0x08) ? 1 : -1];

// IDA_EA: 0x8231E56
// IDA_NAME: _ZN17stGrowthEquipmentC2Ev
// DEMANGLED: stGrowthEquipment::stGrowthEquipment(void)
stGrowthEquipment::stGrowthEquipment()
    : ParamBase() {
    result = 0;
    item_space = 0;
    slot = 0;
    timepiece = 0;
    reserved_14 = 0;
}

// IDA_EA: 0x824648C
// IDA_NAME: _ZN17stGrowthEquipmentD2Ev
// DEMANGLED: stGrowthEquipment::~stGrowthEquipment()
stGrowthEquipment::~stGrowthEquipment() {}

// IDA_EA: 0x82464BC
// IDA_NAME: _ZN17stGrowthEquipmentD0Ev
// DEMANGLED: stGrowthEquipment::~stGrowthEquipment() deleting destructor
void stGrowthEquipment_deleting_destructor(stGrowthEquipment* self) {
    if (self) {
        self->~stGrowthEquipment();
        operator delete(self);
    }
}

// IDA_EA: 0x8231EA4
// IDA_NAME: _ZN16stGrowthCreatureC2Ev
// DEMANGLED: stGrowthCreature::stGrowthCreature(void)
stGrowthCreature::stGrowthCreature()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x824643E
// IDA_NAME: _ZN16stGrowthCreatureD2Ev
// DEMANGLED: stGrowthCreature::~stGrowthCreature()
stGrowthCreature::~stGrowthCreature() {}

// IDA_EA: 0x824646E
// IDA_NAME: _ZN16stGrowthCreatureD0Ev
// DEMANGLED: stGrowthCreature::~stGrowthCreature() deleting destructor
void stGrowthCreature_deleting_destructor(stGrowthCreature* self) {
    if (self) {
        self->~stGrowthCreature();
        operator delete(self);
    }
}

// IDA_EA: 0x823111A
// IDA_NAME: _ZN15stHatchCreatureC2Ev
// DEMANGLED: stHatchCreature::stHatchCreature(void)
stHatchCreature::stHatchCreature()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247912
// IDA_NAME: _ZN15stHatchCreatureD2Ev
// DEMANGLED: stHatchCreature::~stHatchCreature()
stHatchCreature::~stHatchCreature() {}

// IDA_EA: 0x8247942
// IDA_NAME: _ZN15stHatchCreatureD0Ev
// DEMANGLED: stHatchCreature::~stHatchCreature() deleting destructor
void stHatchCreature_deleting_destructor(stHatchCreature* self) {
    if (self) {
        self->~stHatchCreature();
        operator delete(self);
    }
}

// IDA_EA: 0x82310F4
// IDA_NAME: _ZN16stRenameCreatureC2Ev
// DEMANGLED: stRenameCreature::stRenameCreature(void)
stRenameCreature::stRenameCreature()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247960
// IDA_NAME: _ZN16stRenameCreatureD2Ev
// DEMANGLED: stRenameCreature::~stRenameCreature()
stRenameCreature::~stRenameCreature() {}

// IDA_EA: 0x8247990
// IDA_NAME: _ZN16stRenameCreatureD0Ev
// DEMANGLED: stRenameCreature::~stRenameCreature() deleting destructor
void stRenameCreature_deleting_destructor(stRenameCreature* self) {
    if (self) {
        self->~stRenameCreature();
        operator delete(self);
    }
}

// IDA_EA: 0x82313AA
// IDA_NAME: _ZN17stGuildCargoParamC2Ev
// DEMANGLED: stGuildCargoParam::stGuildCargoParam(void)
stGuildCargoParam::stGuildCargoParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247432
// IDA_NAME: _ZN17stGuildCargoParamD2Ev
// DEMANGLED: stGuildCargoParam::~stGuildCargoParam()
stGuildCargoParam::~stGuildCargoParam() {}

// IDA_EA: 0x8247462
// IDA_NAME: _ZN17stGuildCargoParamD0Ev
// DEMANGLED: stGuildCargoParam::~stGuildCargoParam() deleting destructor
void stGuildCargoParam_deleting_destructor(stGuildCargoParam* self) {
    if (self) {
        self->~stGuildCargoParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82313F6
// IDA_NAME: _ZN25stGuildCargoPushItemParamC2Ev
// DEMANGLED: stGuildCargoPushItemParam::stGuildCargoPushItemParam(void)
stGuildCargoPushItemParam::stGuildCargoPushItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247396
// IDA_NAME: _ZN25stGuildCargoPushItemParamD2Ev
// DEMANGLED: stGuildCargoPushItemParam::~stGuildCargoPushItemParam()
stGuildCargoPushItemParam::~stGuildCargoPushItemParam() {}

// IDA_EA: 0x82473C6
// IDA_NAME: _ZN25stGuildCargoPushItemParamD0Ev
// DEMANGLED: stGuildCargoPushItemParam::~stGuildCargoPushItemParam() deleting destructor
void stGuildCargoPushItemParam_deleting_destructor(
    stGuildCargoPushItemParam* self) {
    if (self) {
        self->~stGuildCargoPushItemParam();
        operator delete(self);
    }
}

// IDA_EA: 0x823141C
// IDA_NAME: _ZN24stGuildCargoPopItemParamC2Ev
// DEMANGLED: stGuildCargoPopItemParam::stGuildCargoPopItemParam(void)
stGuildCargoPopItemParam::stGuildCargoPopItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247348
// IDA_NAME: _ZN24stGuildCargoPopItemParamD2Ev
// DEMANGLED: stGuildCargoPopItemParam::~stGuildCargoPopItemParam()
stGuildCargoPopItemParam::~stGuildCargoPopItemParam() {}

// IDA_EA: 0x8247378
// IDA_NAME: _ZN24stGuildCargoPopItemParamD0Ev
// DEMANGLED: stGuildCargoPopItemParam::~stGuildCargoPopItemParam() deleting destructor
void stGuildCargoPopItemParam_deleting_destructor(
    stGuildCargoPopItemParam* self) {
    if (self) {
        self->~stGuildCargoPopItemParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231442
// IDA_NAME: _ZN25stGuildCargoMoveItemParamC2Ev
// DEMANGLED: stGuildCargoMoveItemParam::stGuildCargoMoveItemParam(void)
stGuildCargoMoveItemParam::stGuildCargoMoveItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82472FA
// IDA_NAME: _ZN25stGuildCargoMoveItemParamD2Ev
// DEMANGLED: stGuildCargoMoveItemParam::~stGuildCargoMoveItemParam()
stGuildCargoMoveItemParam::~stGuildCargoMoveItemParam() {}

// IDA_EA: 0x824732A
// IDA_NAME: _ZN25stGuildCargoMoveItemParamD0Ev
// DEMANGLED: stGuildCargoMoveItemParam::~stGuildCargoMoveItemParam() deleting destructor
void stGuildCargoMoveItemParam_deleting_destructor(
    stGuildCargoMoveItemParam* self) {
    if (self) {
        self->~stGuildCargoMoveItemParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82313D0
// IDA_NAME: _ZN24stGuildCargoHistoryParamC2Ev
// DEMANGLED: stGuildCargoHistoryParam::stGuildCargoHistoryParam(void)
stGuildCargoHistoryParam::stGuildCargoHistoryParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82473E4
// IDA_NAME: _ZN24stGuildCargoHistoryParamD2Ev
// DEMANGLED: stGuildCargoHistoryParam::~stGuildCargoHistoryParam()
stGuildCargoHistoryParam::~stGuildCargoHistoryParam() {}

// IDA_EA: 0x8247414
// IDA_NAME: _ZN24stGuildCargoHistoryParamD0Ev
// DEMANGLED: stGuildCargoHistoryParam::~stGuildCargoHistoryParam() deleting destructor
void stGuildCargoHistoryParam_deleting_destructor(
    stGuildCargoHistoryParam* self) {
    if (self) {
        self->~stGuildCargoHistoryParam();
        operator delete(self);
    }
}

// C06 migration marker: CreatureCommonScript and CreatureScript value bodies
// are supplied solely by DataManager/PVF sources.
