#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_ABI_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_ABI_H_

#include <stddef.h>
#include <stdint.h>


// DB_ReloadAutoPunishRuleHackType::dispatch @0x842D16C reserves this exact
// signal-9 Inter record before it queues DISPATCH_Q.  The fixed transport
// layout is shared by that B/DB producer and Inter_AutoPunishRuleHackType;
// it is a wire record, not a substitute for the manager's in-memory state.
#pragma pack(push, 1)
struct SIG_AUTO_PUNISH_RULE_HACK_TYPE_ENTRY {
    uint16_t hack_type;       // +0x00
    uint16_t count;           // +0x02
    uint16_t hack_sub_type;   // +0x04
    uint16_t hack_sub_count;  // +0x06
    uint16_t apply_flag;      // +0x08
    uint8_t reserved_0a[2];   // +0x0a
    uint32_t ip_count;        // +0x0c
    uint32_t etc_low;         // +0x10
};

struct SIG_AUTO_PUNISH_RULE_HACK_TYPE {
    uint32_t total_count;     // +0x00
    uint64_t total_etc;       // +0x04
    uint16_t rule_count;      // +0x0c
    uint8_t reserved_0e[2];   // +0x0e
    SIG_AUTO_PUNISH_RULE_HACK_TYPE_ENTRY entries[500];  // +0x10
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUTO_PUNISH_RULE_HACK_TYPE_ENTRY) == 0x14u,
              "old AutoPunish hack-type entry must stay 20 bytes");
static_assert(offsetof(SIG_AUTO_PUNISH_RULE_HACK_TYPE, entries) == 0x10u,
              "old AutoPunish hack-type entries begin at +0x10");
static_assert(sizeof(SIG_AUTO_PUNISH_RULE_HACK_TYPE) == 0x2720u,
              "old AutoPunish hack-type signal-9 record must stay 0x2720 bytes");


#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_ABI_H_
