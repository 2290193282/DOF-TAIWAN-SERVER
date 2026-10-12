#include "old_game/social/party/party.h"

// IDA_EA: 0x8151A56
// IDA_NAME: _ZN19STGainedEachExpDataC2Ev
// DEMANGLED: STGainedEachExpData::STGainedEachExpData(void)
STGainedEachExpData::STGainedEachExpData()
    : gained_exp(0),
      exp_scale(1.0f) {
    // member_level_gap is intentionally caller-supplied.  IDA 0x8151A56
    // performs no write to the third dword.
}
