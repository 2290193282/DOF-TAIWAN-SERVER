#ifndef OLDSRV_SECURITY_CLIENT_HACK_REPORT_DISPATCHERS_H_
#define OLDSRV_SECURITY_CLIENT_HACK_REPORT_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 111 / REPORT_CLIENT_4_HACK.  The old 606-entry PacketDispatcher table
// is not active yet, so this exact state-0 provider intentionally remains
// unregistered.
class DisPatcher_ReportClient4Hack
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595F90
    // IDA_NAME: _ZN28DisPatcher_ReportClient4HackC2Ev
    // DEMANGLED: DisPatcher_ReportClient4Hack::DisPatcher_ReportClient4Hack(void)
    DisPatcher_ReportClient4Hack();

    // IDA_EA: 0x8242D18
    // IDA_NAME: _ZN28DisPatcher_ReportClient4HackD2Ev
    // DEMANGLED: DisPatcher_ReportClient4Hack::~DisPatcher_ReportClient4Hack()
    // IDA_EA: 0x8242D48
    // IDA_NAME: _ZN28DisPatcher_ReportClient4HackD0Ev
    // DEMANGLED: DisPatcher_ReportClient4Hack::~DisPatcher_ReportClient4Hack()
    virtual ~DisPatcher_ReportClient4Hack();

    // IDA_EA: 0x82041A0
    // IDA_NAME: _ZN28DisPatcher_ReportClient4Hack12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_ReportClient4Hack::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233738
    // IDA_NAME: _ZN28DisPatcher_ReportClient4Hack4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_ReportClient4Hack::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233742
    // IDA_NAME: _ZN28DisPatcher_ReportClient4Hack7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_ReportClient4Hack::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD 114 / CODE_CHECKSUM.  The old dispatch reads a raw 32-byte checksum
// value; the canonical source begins the equivalent no-op-dtor value object's
// lifetime before filling its bytes.  Only the base dispatcher has state.
class Dispatcher_CodeCheckSum
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595FE4
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSumC2Ev
    // DEMANGLED: Dispatcher_CodeCheckSum::Dispatcher_CodeCheckSum(void)
    Dispatcher_CodeCheckSum();

    // IDA_EA: 0x8242C2E
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSumD2Ev
    // DEMANGLED: Dispatcher_CodeCheckSum::~Dispatcher_CodeCheckSum()
    // IDA_EA: 0x8242C5E
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSumD0Ev
    // DEMANGLED: Dispatcher_CodeCheckSum::~Dispatcher_CodeCheckSum()
    virtual ~Dispatcher_CodeCheckSum();

    // IDA_EA: 0x82044B0
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSum12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CodeCheckSum::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233774
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSum4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CodeCheckSum::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823377E
    // IDA_NAME: _ZN23Dispatcher_CodeCheckSum7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CodeCheckSum::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ReportClient4Hack) == 0x04u,
              "CMD111 dispatcher must retain the raw four-byte i386 base");
static_assert(sizeof(Dispatcher_CodeCheckSum) == 0x04u,
              "CMD114 dispatcher must retain the raw four-byte i386 base");
#endif

#endif  // OLDSRV_SECURITY_CLIENT_HACK_REPORT_DISPATCHERS_H_
