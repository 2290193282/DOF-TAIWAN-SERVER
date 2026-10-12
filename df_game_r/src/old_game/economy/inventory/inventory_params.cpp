#include "old_game/economy/inventory/inventory_params.h"

#include <new>


typedef char stOnUseEquipParam_Size_Must_Be_0x0c[
    (sizeof(stOnUseEquipParam) == 0x0c) ? 1 : -1];
typedef char stOnGetUserInfoParam_Size_Must_Be_0x08[
    (sizeof(stOnGetUserInfoParam) == 0x08) ? 1 : -1];
typedef char stRepairEquipParam_Size_Must_Be_0x10[
    (sizeof(stRepairEquipParam) == 0x10) ? 1 : -1];
typedef char stImageCommunicationEquipmentUse_Size_Must_Be_0x10[
    (sizeof(stImageCommunicationEquipmentUse) == 0x10) ? 1 : -1];

// IDA_EA: 0x8230AE4
// IDA_NAME: _ZN17stOnUseEquipParamC2Ev
// DEMANGLED: stOnUseEquipParam::stOnUseEquipParam(void)
stOnUseEquipParam::stOnUseEquipParam()
    : ParamBase(),
      result(0),
      slot(0),
      padding_09() {}

// IDA_EA: 0x824836E
// IDA_NAME: _ZN17stOnUseEquipParamD2Ev
// DEMANGLED: stOnUseEquipParam::~stOnUseEquipParam()
stOnUseEquipParam::~stOnUseEquipParam() {}

// IDA_EA: 0x824839E
// IDA_NAME: _ZN17stOnUseEquipParamD0Ev
// DEMANGLED: stOnUseEquipParam::~stOnUseEquipParam() deleting destructor
void stOnUseEquipParam_deleting_destructor(stOnUseEquipParam* self) {
    if (self) {
        self->~stOnUseEquipParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230C36
// IDA_NAME: _ZN20stOnGetUserInfoParamC2Ev
// DEMANGLED: stOnGetUserInfoParam::stOnGetUserInfoParam(void)
stOnGetUserInfoParam::stOnGetUserInfoParam()
    : ParamBase(),
      target_id(0),
      detail_type(0),
      padding_07(0) {}

// IDA_EA: 0x824819A
// IDA_NAME: _ZN20stOnGetUserInfoParamD2Ev
// DEMANGLED: stOnGetUserInfoParam::~stOnGetUserInfoParam()
stOnGetUserInfoParam::~stOnGetUserInfoParam() {}

// IDA_EA: 0x82481CA
// IDA_NAME: _ZN20stOnGetUserInfoParamD0Ev
// DEMANGLED: stOnGetUserInfoParam::~stOnGetUserInfoParam() deleting destructor
void stOnGetUserInfoParam_deleting_destructor(stOnGetUserInfoParam* self) {
    if (self) {
        self->~stOnGetUserInfoParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230D72
// IDA_NAME: _ZN18stRepairEquipParamC2Ev
// DEMANGLED: stRepairEquipParam::stRepairEquipParam(void)
stRepairEquipParam::stRepairEquipParam()
    : ParamBase(),
      result(0),
      inven_type(0),
      padding_09(0),
      slot(0),
      repair_cost(0),
      padding_0e() {}

// IDA_EA: 0x8247F78
// IDA_NAME: _ZN18stRepairEquipParamD2Ev
// DEMANGLED: stRepairEquipParam::~stRepairEquipParam()
stRepairEquipParam::~stRepairEquipParam() {}

// IDA_EA: 0x8247FA8
// IDA_NAME: _ZN18stRepairEquipParamD0Ev
// DEMANGLED: stRepairEquipParam::~stRepairEquipParam() deleting destructor
void stRepairEquipParam_deleting_destructor(stRepairEquipParam* self) {
    if (self) {
        self->~stRepairEquipParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231B90
// IDA_NAME: _ZN32stImageCommunicationEquipmentUseC2Ev
// DEMANGLED: stImageCommunicationEquipmentUse::stImageCommunicationEquipmentUse(void)
stImageCommunicationEquipmentUse::stImageCommunicationEquipmentUse()
    : ParamBase(),
      result(0),
      money_cost(0),
      cur_money(0) {}

// IDA_EA: 0x8246866
// IDA_NAME: _ZN32stImageCommunicationEquipmentUseD2Ev
// DEMANGLED: stImageCommunicationEquipmentUse::~stImageCommunicationEquipmentUse()
stImageCommunicationEquipmentUse::~stImageCommunicationEquipmentUse() {}

// IDA_EA: 0x8246896
// IDA_NAME: _ZN32stImageCommunicationEquipmentUseD0Ev
// DEMANGLED: stImageCommunicationEquipmentUse::~stImageCommunicationEquipmentUse() deleting destructor
void stImageCommunicationEquipmentUse_deleting_destructor(
    stImageCommunicationEquipmentUse* self) {
    if (self) {
        self->~stImageCommunicationEquipmentUse();
        operator delete(self);
    }
}
