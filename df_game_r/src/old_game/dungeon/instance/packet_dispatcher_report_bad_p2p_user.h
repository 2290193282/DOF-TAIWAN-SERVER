#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REPORT_BAD_P2P_USER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REPORT_BAD_P2P_USER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 52.  This is a direct dispatcher: the old dispatch_sig reads the one
// request byte itself, while read/process are original no-op virtual slots.
class Dispatcher_ReportBadP2PUser
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595AC0
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserC2Ev
    // DEMANGLED: Dispatcher_ReportBadP2PUser::Dispatcher_ReportBadP2PUser(void)
    Dispatcher_ReportBadP2PUser();

    // IDA_EA: 0x8243774
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserD2Ev
    // DEMANGLED: Dispatcher_ReportBadP2PUser::~Dispatcher_ReportBadP2PUser()
    // IDA_EA: 0x82437A4
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserD0Ev
    // DEMANGLED: Dispatcher_ReportBadP2PUser::~Dispatcher_ReportBadP2PUser()
    virtual ~Dispatcher_ReportBadP2PUser();

    // IDA_EA: 0x81FD0A8
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ReportBadP2PUser::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823347C
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ReportBadP2PUser::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233486
    // IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ReportBadP2PUser::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ReportBadP2PUser) == 0x04u,
              "CMD52 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REPORT_BAD_P2P_USER_H_
