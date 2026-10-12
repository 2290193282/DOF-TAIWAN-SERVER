#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_COUNTER_ABI_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_COUNTER_ABI_H_

#include <stddef.h>
#include <stdint.h>


// Inter_SetIPCount @0x84DCB18 and Inter_SetFullIPCount @0x84DCBEA both
// examine only these two bytes before walking their fixed-stride records.
#pragma pack(push, 1)
struct SIG_AUTO_PUNISH_COUNTER_HEADER {
    uint8_t unknown_00[10];  // +0x00, not consumed by these receivers
    uint8_t keep_existing;   // +0x0a: zero calls the corresponding reset
    uint8_t entry_count;     // +0x0b
};

struct SIG_AUTO_PUNISH_IP_COUNTER_ENTRY {
    uint16_t hack_type;      // +0x00
    uint16_t hack_sub_type;  // +0x02
    char c_class_ip[12];     // +0x04, raw caller passes this address to C2
    uint32_t ip_count;       // +0x10
};

struct SIG_AUTO_PUNISH_FULL_IP_COUNTER_ENTRY {
    uint16_t hack_type;      // +0x00
    uint16_t hack_sub_type;  // +0x02
    char full_ip[16];        // +0x04
    uint32_t ip_count;       // +0x14
};

// Inter_SetARSInfo @0x84E09CE uses a signed dword count at +0x0a, one
// continuation byte at +0x0e, then twelve-byte records beginning at +0x0f.
struct SIG_AUTO_PUNISH_ARS_HEADER {
    uint8_t unknown_00[10];  // +0x00, not consumed by this receiver
    int32_t entry_count;     // +0x0a, checked with signed <= 100
    int8_t keep_existing;    // +0x0e: zero calls manager reset
};

struct SIG_AUTO_PUNISH_ARS_ENTRY {
    uint16_t hack_type;       // +0x00; zero selects total-trade rule
    uint16_t count;           // +0x02
    uint16_t hack_sub_type;   // +0x04
    uint16_t hack_sub_count;  // +0x06
    uint8_t apply_flag;       // +0x08
    uint8_t ip_count;         // +0x09
    uint16_t etc_low;         // +0x0a; total-trade high dword is zero
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUTO_PUNISH_COUNTER_HEADER) == 0x0cu,
              "old IP-counter header must stay 12 bytes");
static_assert(sizeof(SIG_AUTO_PUNISH_IP_COUNTER_ENTRY) == 0x14u,
              "old class-IP counter entry must stay 20 bytes");
static_assert(sizeof(SIG_AUTO_PUNISH_FULL_IP_COUNTER_ENTRY) == 0x18u,
              "old full-IP counter entry must stay 24 bytes");
static_assert(offsetof(SIG_AUTO_PUNISH_ARS_HEADER, keep_existing) == 0x0eu,
              "old ARS continuation byte must stay at +0x0e");
static_assert(sizeof(SIG_AUTO_PUNISH_ARS_HEADER) == 0x0fu,
              "old ARS records must start at +0x0f");
static_assert(sizeof(SIG_AUTO_PUNISH_ARS_ENTRY) == 0x0cu,
              "old ARS entry must stay 12 bytes");


#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_COUNTER_ABI_H_
