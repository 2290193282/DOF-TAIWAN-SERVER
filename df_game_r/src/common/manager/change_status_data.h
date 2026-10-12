#ifndef OLDSRV_GLOBAL_CHANGE_STATUS_DATA_H_
#define OLDSRV_GLOBAL_CHANGE_STATUS_DATA_H_

#include <stdint.h>

#include <string>

#include "old_data/PVF/common/script_value.h"


// IDA_EA: ChangeStatusData @0x89C2DD8; VariableStat @0x8366262;
// getChangeStatusType @0x8892B0D; scanChangeStatusData @0x88BDA1C.
// IDA unit: 99_misc_global/ChangeStatusData. Old i386 size: 0x0C.
struct ChangeStatusData {
    ChangeStatusData();
    ChangeStatusData(const ChangeStatusData& other);

    ChangeStatusData& operator=(const ChangeStatusData& other);

    int32_t change_status_type;  // +0x00, default 77 in old constructor.
    int32_t is_percent;          // +0x04, set from "%" marker.
    float value;                 // +0x08, assigned from ScanInt result.
};

// IDA unit: 99_misc_global/VariableStat. Old i386 size: 0x20.
struct VariableStat {
    VariableStat();
    VariableStat(const VariableStat& other);
    ~VariableStat();

    VariableStat& operator=(const VariableStat& other);

    ChangeStatusData status;  // +0x00
    ScriptValue script_value; // +0x0C
};

int getChangeStatusType(std::string& value);
int scanChangeStatusData(ChangeStatusData& value);
int scanVariableStat(VariableStat& value);


#endif  // OLDSRV_GLOBAL_CHANGE_STATUS_DATA_H_
