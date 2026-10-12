#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_DONATE_GUILD_FUND_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_DONATE_GUILD_FUND_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD347 carries one signed money dword after the 13-byte packed header. The
// old template constructs this request directly; no message C2 was emitted.
#pragma pack(push, 1)
struct MSG_DONATE_GUILD_FUND : public MSG_BASE {
    int32_t fund_amount;  // +0x0d
};
#pragma pack(pop)

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_DONATE_GUILD_FUND) == 0x11u,
              "CMD347 request must retain its old packed extent");
static_assert(offsetof(MSG_DONATE_GUILD_FUND, fund_amount) == 0x0du,
              "CMD347 fund amount must remain at +0x0d");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// This provider is deliberately not inserted into PacketDispatcher's root
// table until the original 606-slot registration transaction is complete.
class Dispatcher_DonateGuildFund
    : public IPacketDispatcher<MSG_DONATE_GUILD_FUND,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859745E
    // IDA_NAME: _ZN26Dispatcher_DonateGuildFundC2Ev
    // DEMANGLED: Dispatcher_DonateGuildFund::Dispatcher_DonateGuildFund(void)
    Dispatcher_DonateGuildFund();

    // IDA_EA: 0x8245292 / 0x82452C2
    // IDA_NAME: _ZN26Dispatcher_DonateGuildFundD2Ev /
    //           _ZN26Dispatcher_DonateGuildFundD0Ev
    // DEMANGLED: Dispatcher_DonateGuildFund::~Dispatcher_DonateGuildFund()
    virtual ~Dispatcher_DonateGuildFund();

    // IDA_EA: 0x81D4920 / 0x81D4970
    // IDA_NAME: _ZN26Dispatcher_DonateGuildFund4readER9PacketBufR8MSG_BASE /
    //           _ZN26Dispatcher_DonateGuildFund7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_DonateGuildFund::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_DonateGuildFund::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DonateGuildFund) == 0x04u,
              "CMD347 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_DONATE_GUILD_FUND_DISPATCHER_H_
