#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_REQUEST_PVP_EXP_OF_WEEK_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_REQUEST_PVP_EXP_OF_WEEK_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 08_quest_skill_growth/Dispatcher_Request_PvpExpOfWeek.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_Request_PvpExpOfWeek
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595B84
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekC2Ev
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::Dispatcher_Request_PvpExpOfWeek(void)
    Dispatcher_Request_PvpExpOfWeek();

    // IDA_EA: 0x82435A0
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekD2Ev
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::~Dispatcher_Request_PvpExpOfWeek()
    // IDA_EA: 0x82435D0
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekD0Ev
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::~Dispatcher_Request_PvpExpOfWeek()
    virtual ~Dispatcher_Request_PvpExpOfWeek();

    // IDA_EA: 0x81FECAA
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82334F4
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82334FE
    // IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Request_PvpExpOfWeek::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Request_PvpExpOfWeek) == 0x04u,
              "CMD69 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_REQUEST_PVP_EXP_OF_WEEK_H_
