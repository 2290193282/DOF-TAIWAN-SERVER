#ifndef OLDSRV_OLD_DATA_DB_CORE_NAT_TYPE_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CORE_NAT_TYPE_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

// IDA payload used by StatisticsNatType::UpdateNatType @0x860C968 and
// DB_UpdateNatType::dispatch @0x841C74E.  It is sent after LogDB signal 131.
#pragma pack(push, 1)
struct SIG_NAT_TYPE {
    int32_t nat_type;        // +0x00
    int32_t nat_type_other;  // +0x04
    int32_t fail_count;      // +0x08
    int32_t success_count;   // +0x0C
    int32_t occ_time;        // +0x10
};
#pragma pack(pop)

static_assert(sizeof(SIG_NAT_TYPE) == 0x14u,
              "SIG_NAT_TYPE must retain the old 20-byte LogDB ABI");
static_assert(offsetof(SIG_NAT_TYPE, nat_type_other) == 0x04u,
              "SIG_NAT_TYPE nat_type_other offset changed");
static_assert(offsetof(SIG_NAT_TYPE, fail_count) == 0x08u,
              "SIG_NAT_TYPE fail_count offset changed");
static_assert(offsetof(SIG_NAT_TYPE, success_count) == 0x0Cu,
              "SIG_NAT_TYPE success_count offset changed");
static_assert(offsetof(SIG_NAT_TYPE, occ_time) == 0x10u,
              "SIG_NAT_TYPE occ_time offset changed");

#endif  // OLDSRV_OLD_DATA_DB_CORE_NAT_TYPE_PAYLOAD_H_
