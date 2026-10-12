#include "old_data/PVF/dungeon/dungeon_script_value_types.h"

// IDA_EA: 0x88F9346
// IDA_NAME: _ZN12RecoveryInfoD2Ev
// DEMANGLED: RecoveryInfo::~RecoveryInfo()
RecoveryInfo::~RecoveryInfo() {}

// IDA_EA: 0x88F2E84
// IDA_NAME: _ZN17RecoveryStairInfoC2Ev
// DEMANGLED: RecoveryStairInfo::RecoveryStairInfo(void)
RecoveryStairInfo::RecoveryStairInfo() : recovery_infos() {}

// IDA_EA: 0x88F627E
// IDA_NAME: _ZN17RecoveryStairInfoC2ERKS_
// DEMANGLED: RecoveryStairInfo::RecoveryStairInfo(RecoveryStairInfo const&)
RecoveryStairInfo::RecoveryStairInfo(const RecoveryStairInfo& rhs)
    : value_00(rhs.value_00),
      value_04(rhs.value_04),
      value_08(rhs.value_08),
      recovery_infos(rhs.recovery_infos) {}

// IDA_EA: 0x83DC7D2
// IDA_NAME: _ZN17RecoveryStairInfoD2Ev
// DEMANGLED: RecoveryStairInfo::~RecoveryStairInfo()
RecoveryStairInfo::~RecoveryStairInfo() {}

// IDA_EA: 0x88F6344
// IDA_NAME: _ZN17RecoveryStairInfoaSERKS_
// DEMANGLED: RecoveryStairInfo& RecoveryStairInfo::operator=(RecoveryStairInfo const&)
RecoveryStairInfo& RecoveryStairInfo::operator=(const RecoveryStairInfo& rhs) {
    value_00 = rhs.value_00;
    value_04 = rhs.value_04;
    value_08 = rhs.value_08;
    recovery_infos = rhs.recovery_infos;
    return *this;
}
