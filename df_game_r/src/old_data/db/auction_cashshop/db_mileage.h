#ifndef OLDSRV_DB_DB_MILEAGE_H_
#define OLDSRV_DB_DB_MILEAGE_H_

#include <stdint.h>

// IDA_EA: 0x8453F5C, 0x869F42E.
// IDA unit: 13_database_dispatch/SIG_MILEAGE_STRUCT.
#pragma pack(push, 1)
struct SIG_MILEAGE_STRUCT {
    uint32_t m_id;
    uint32_t charac_no;
    int32_t miles;
};
#pragma pack(pop)

static_assert(sizeof(SIG_MILEAGE_STRUCT) == 0x0Cu,
              "SIG_MILEAGE_STRUCT IDA ABI size must stay 0x0C");

#endif  // OLDSRV_DB_DB_MILEAGE_H_
