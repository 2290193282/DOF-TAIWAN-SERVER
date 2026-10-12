#include "common/manager/server_parameter_script_abi.h"

#include <string.h>

// IDA_EA: 0x8A8CC64
// IDA_NAME: _ZN23stBurningFatigueParam_tC2Ev
// DEMANGLED: stBurningFatigueParam_t::stBurningFatigueParam_t(void)
stBurningFatigueParam_t::stBurningFatigueParam_t()
    : fatigue(0),
      pad_02(0),
      bonus_exp_rate(0),
      refill_fatigue(0),
      pad_0a(0),
      value_0c(0),
      value_10(1.0f),
      value_14(0) {
    memset(raw_18, 0, sizeof(raw_18));
}

// IDA_EA: 0x8A91BB0
// IDA_NAME: _ZN23stBurningFatigueParam_tD2Ev
// DEMANGLED: stBurningFatigueParam_t::~stBurningFatigueParam_t()
stBurningFatigueParam_t::~stBurningFatigueParam_t() {}
