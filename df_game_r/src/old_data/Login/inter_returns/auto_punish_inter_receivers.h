#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_AUTO_PUNISH_INTER_RECEIVERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_AUTO_PUNISH_INTER_RECEIVERS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_data/Login/security/auto_punish_counter_abi.h"
#include "old_data/Login/security/auto_punish_hack_type_abi.h"


class CUser;


// DB_ReloadAutoPunishRuleBackIP::dispatch @0x842CDF6 reserves and writes this
// exact 0x3e84-byte Inter reply before queueing signal 8.  The two otherwise
// unused bytes in each header/entry remain named individual reserved fields;
// no untyped transport buffer is used for this fixed old record.
#pragma pack(push, 1)
struct SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY {
    uint32_t c_class_ip;  // +0x00, inet_addr("a.b.c.0")
    uint8_t start_ip;     // +0x04
    uint8_t end_ip;       // +0x05
    uint8_t reserved_06;  // +0x06
    uint8_t reserved_07;  // +0x07
};

struct SIG_AUTO_PUNISH_RULE_BLACK_IP {
    uint16_t entry_count;  // +0x00
    uint8_t reserved_02;   // +0x02
    uint8_t reserved_03;   // +0x03
    SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY entries[2000];  // +0x04
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY) == 0x08u,
              "black-IP rule entry ABI must stay eight bytes");
static_assert(sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP) == 0x3e84u,
              "black-IP rule Inter reply ABI must stay 0x3e84 bytes");
static_assert(offsetof(SIG_AUTO_PUNISH_RULE_BLACK_IP, entries) == 0x04u,
              "black-IP rule entries must begin at +0x04");


class Inter_AutoPunishRuleBlackIP : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDB98
    // IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPC2Ev
    // DEMANGLED: Inter_AutoPunishRuleBlackIP::Inter_AutoPunishRuleBlackIP(void)
    Inter_AutoPunishRuleBlackIP();

    // IDA_EA: 0x84F8F04
    // IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPD2Ev
    // IDA_EA: 0x84F8F34
    // IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPD0Ev
    // DEMANGLED: Inter_AutoPunishRuleBlackIP::~Inter_AutoPunishRuleBlackIP()
    virtual ~Inter_AutoPunishRuleBlackIP();

    // IDA_EA: 0x84D422A
    // IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIP12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


class Inter_AutoPunishRuleHackType : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDBB4
    // IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeC2Ev
    // DEMANGLED: Inter_AutoPunishRuleHackType::Inter_AutoPunishRuleHackType(void)
    Inter_AutoPunishRuleHackType();

    // IDA_EA: 0x84F8EB6
    // IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeD2Ev
    // IDA_EA: 0x84F8EE6
    // IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeD0Ev
    // DEMANGLED: Inter_AutoPunishRuleHackType::~Inter_AutoPunishRuleHackType()
    virtual ~Inter_AutoPunishRuleHackType();

    // IDA_EA: 0x84D4440
    // IDA_NAME: _ZN28Inter_AutoPunishRuleHackType12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_AutoPunishRuleHackType::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


class Inter_SetIPCount : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE2D0
    // IDA_NAME: _ZN16Inter_SetIPCountC2Ev
    // DEMANGLED: Inter_SetIPCount::Inter_SetIPCount(void)
    Inter_SetIPCount();

    // IDA_EA: 0x84F7A9A
    // IDA_NAME: _ZN16Inter_SetIPCountD2Ev
    // IDA_EA: 0x84F7ACA
    // IDA_NAME: _ZN16Inter_SetIPCountD0Ev
    // DEMANGLED: Inter_SetIPCount::~Inter_SetIPCount()
    virtual ~Inter_SetIPCount();

    // IDA_EA: 0x84DCB18
    // IDA_NAME: _ZN16Inter_SetIPCount12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SetIPCount::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


class Inter_SetFullIPCount : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE2EC
    // IDA_NAME: _ZN20Inter_SetFullIPCountC2Ev
    // DEMANGLED: Inter_SetFullIPCount::Inter_SetFullIPCount(void)
    Inter_SetFullIPCount();

    // IDA_EA: 0x84F7A4C
    // IDA_NAME: _ZN20Inter_SetFullIPCountD2Ev
    // IDA_EA: 0x84F7A7C
    // IDA_NAME: _ZN20Inter_SetFullIPCountD0Ev
    // DEMANGLED: Inter_SetFullIPCount::~Inter_SetFullIPCount()
    virtual ~Inter_SetFullIPCount();

    // IDA_EA: 0x84DCBEA
    // IDA_NAME: _ZN20Inter_SetFullIPCount12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SetFullIPCount::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


class Inter_SetARSInfo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE634
    // IDA_NAME: _ZN16Inter_SetARSInfoC2Ev
    // DEMANGLED: Inter_SetARSInfo::Inter_SetARSInfo(void)
    Inter_SetARSInfo();

    // IDA_EA: 0x84F7128
    // IDA_NAME: _ZN16Inter_SetARSInfoD2Ev
    // IDA_EA: 0x84F7158
    // IDA_NAME: _ZN16Inter_SetARSInfoD0Ev
    // DEMANGLED: Inter_SetARSInfo::~Inter_SetARSInfo()
    virtual ~Inter_SetARSInfo();

    // IDA_EA: 0x84E09CE
    // IDA_NAME: _ZN16Inter_SetARSInfo12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SetARSInfo::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#if defined(__i386__)
static_assert(sizeof(Inter_AutoPunishRuleBlackIP) == 0x04u,
              "Inter_AutoPunishRuleBlackIP must retain its base-only i386 ABI");
static_assert(sizeof(Inter_AutoPunishRuleHackType) == 0x04u,
              "Inter_AutoPunishRuleHackType must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SetIPCount) == 0x04u,
              "Inter_SetIPCount must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SetFullIPCount) == 0x04u,
              "Inter_SetFullIPCount must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SetARSInfo) == 0x04u,
              "Inter_SetARSInfo must retain its base-only i386 ABI");
#endif


#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_AUTO_PUNISH_INTER_RECEIVERS_H_
