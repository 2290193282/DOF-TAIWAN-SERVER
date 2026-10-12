#ifndef OLDSRV_PROTOCOL_OVERFLOW_INFO_DISPATCHER_H_
#define OLDSRV_PROTOCOL_OVERFLOW_INFO_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// The statistic packet constructed by CMD 220.  The old stack allocation
// places the two parsed body values immediately after its 10-byte PacketHeader
// and sends exactly 0x0D bytes through the statistic proxy.
#pragma pack(push, 1)
struct Packet_Overflow_Statistic_Add {
    PacketHeader header;
    uint8_t raw_0a;
    uint16_t raw_0b;

    // IDA_EA: 0x822BDF0
    // IDA_NAME: _ZN29Packet_Overflow_Statistic_AddC2Ev
    // DEMANGLED: Packet_Overflow_Statistic_Add::Packet_Overflow_Statistic_Add(void)
    Packet_Overflow_Statistic_Add();
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0Au,
              "Packet_Overflow_Statistic_Add requires a 10-byte PacketHeader");
static_assert(sizeof(Packet_Overflow_Statistic_Add) == 0x0Du,
              "CMD220 overflow statistic packet must stay 13 bytes");
static_assert(offsetof(Packet_Overflow_Statistic_Add, header) == 0x00u &&
                  offsetof(Packet_Overflow_Statistic_Add, raw_0a) == 0x0Au &&
                  offsetof(Packet_Overflow_Statistic_Add, raw_0b) == 0x0Bu,
              "CMD220 overflow statistic packet offsets changed");

// CMD 220 / OVERFLOW_INFO.  This exact state-0 handler is deliberately not
// inserted into PacketDispatcher until the original complete 606-slot root is
// restored.
class Dispatcher_Overflow_Info
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596AB8
    // IDA_NAME: _ZN24Dispatcher_Overflow_InfoC2Ev
    // DEMANGLED: Dispatcher_Overflow_Info::Dispatcher_Overflow_Info(void)
    Dispatcher_Overflow_Info();

    // IDA_EA: 0x82412E4
    // IDA_NAME: _ZN24Dispatcher_Overflow_InfoD2Ev
    // DEMANGLED: Dispatcher_Overflow_Info::~Dispatcher_Overflow_Info()
    // IDA_EA: 0x8241314
    // IDA_NAME: _ZN24Dispatcher_Overflow_InfoD0Ev
    // DEMANGLED: Dispatcher_Overflow_Info::~Dispatcher_Overflow_Info()
    virtual ~Dispatcher_Overflow_Info();

    // IDA_EA: 0x821E4B6
    // IDA_NAME: _ZN24Dispatcher_Overflow_Info12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Overflow_Info::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233E66
    // IDA_NAME: _ZN24Dispatcher_Overflow_Info4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Overflow_Info::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233E70
    // IDA_NAME: _ZN24Dispatcher_Overflow_Info7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Overflow_Info::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_OVERFLOW_INFO_DISPATCHER_H_
