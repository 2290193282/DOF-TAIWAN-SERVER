#ifndef OLDSRV_OLD_GAME_INVENTORY_DELETE_ITEM_PARAM_H_
#define OLDSRV_OLD_GAME_INVENTORY_DELETE_ITEM_PARAM_H_

#include <stdint.h>

#include "infra/network/packet/message_base.h"

// IDA unit: 99_misc_global/stDeleteItemParam.  The CMD18 state-3 template
// constructs this exact parameter object before its concrete process call.
struct __attribute__((packed)) stDeleteItemParam : public ParamBase {
    // IDA_EA: 0x8230B38
    // IDA_NAME: _ZN17stDeleteItemParamC2Ev
    // DEMANGLED: stDeleteItemParam::stDeleteItemParam(void)
    stDeleteItemParam();

    // IDA_EA: 0x82482D2
    // IDA_NAME: _ZN17stDeleteItemParamD2Ev
    // DEMANGLED: stDeleteItemParam::~stDeleteItemParam()
    // IDA_EA: 0x8248302
    // IDA_NAME: _ZN17stDeleteItemParamD0Ev
    // DEMANGLED: stDeleteItemParam::~stDeleteItemParam()
    virtual ~stDeleteItemParam();

    int error_;                         // IDA +0x04
    int8_t item_space_;                 // IDA +0x08
    int8_t item_count_;                 // IDA +0x09
    int16_t delete_type_[100];          // IDA +0x0a
    int16_t slots_[100];                // IDA +0xd2
    uint8_t padding_19a_[2];            // IDA +0x19a
    int32_t counts_[100];               // IDA +0x19c
};

#if defined(__i386__)
static_assert(sizeof(stDeleteItemParam) == 0x32cu,
              "CMD18 parameter must retain the old 812-byte ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_DELETE_ITEM_PARAM_H_
