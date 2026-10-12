#include "old_game/economy/inventory/delete_item_param.h"

// IDA_EA: 0x8230B38
// IDA_NAME: _ZN17stDeleteItemParamC2Ev
// DEMANGLED: stDeleteItemParam::stDeleteItemParam(void)
stDeleteItemParam::stDeleteItemParam()
    : ParamBase(), error_(0), item_space_(0), item_count_(0) {
    for (int index = 0; index <= 99; ++index) {
        delete_type_[index] = 0;
        slots_[index] = 0;
        counts_[index] = 0;
    }
}

// IDA_EA: 0x82482D2
// IDA_NAME: _ZN17stDeleteItemParamD2Ev
// DEMANGLED: stDeleteItemParam::~stDeleteItemParam()
// IDA_EA: 0x8248302
// IDA_NAME: _ZN17stDeleteItemParamD0Ev
// DEMANGLED: stDeleteItemParam::~stDeleteItemParam()
stDeleteItemParam::~stDeleteItemParam() {}
