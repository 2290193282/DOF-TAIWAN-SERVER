#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_JOIN_POWER_WAR_DISPATCHER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_JOIN_POWER_WAR_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;


// CMD 195. The old object contains only the state-0 dispatcher base.
class Dispatcher_JoinPowerWar
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85967C4
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWarC2Ev
    // DEMANGLED: Dispatcher_JoinPowerWar::Dispatcher_JoinPowerWar(void)
    Dispatcher_JoinPowerWar();

    // IDA_EA: 0x8241A34
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWarD2Ev
    // DEMANGLED: Dispatcher_JoinPowerWar::~Dispatcher_JoinPowerWar()
    // IDA_EA: 0x8241A64
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWarD0Ev
    // DEMANGLED: Dispatcher_JoinPowerWar::~Dispatcher_JoinPowerWar()
    virtual ~Dispatcher_JoinPowerWar();

    // IDA_EA: 0x821701A
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWar12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_JoinPowerWar::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B32
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWar4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_JoinPowerWar::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B3C
    // IDA_NAME: _ZN23Dispatcher_JoinPowerWar7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_JoinPowerWar::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};


#if defined(__i386__)
static_assert(sizeof(Dispatcher_JoinPowerWar) == 0x04u,
              "CMD195 dispatcher must retain its i386 base-only ABI");
#endif


#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_JOIN_POWER_WAR_DISPATCHER_H_
