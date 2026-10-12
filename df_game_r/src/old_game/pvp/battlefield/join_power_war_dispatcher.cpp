#include "old_game/pvp/battlefield/join_power_war_dispatcher.h"

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/network/packet/packet_buf.h"


// IDA_EA: 0x85967C4
// IDA_NAME: _ZN23Dispatcher_JoinPowerWarC2Ev
// DEMANGLED: Dispatcher_JoinPowerWar::Dispatcher_JoinPowerWar(void)
Dispatcher_JoinPowerWar::Dispatcher_JoinPowerWar()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241A34
// IDA_NAME: _ZN23Dispatcher_JoinPowerWarD2Ev
// DEMANGLED: Dispatcher_JoinPowerWar::~Dispatcher_JoinPowerWar()
// IDA_EA: 0x8241A64
// IDA_NAME: _ZN23Dispatcher_JoinPowerWarD0Ev
// DEMANGLED: Dispatcher_JoinPowerWar::~Dispatcher_JoinPowerWar()
Dispatcher_JoinPowerWar::~Dispatcher_JoinPowerWar() {}

// IDA_EA: 0x821701A
// IDA_NAME: _ZN23Dispatcher_JoinPowerWar12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_JoinPowerWar::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_JoinPowerWar::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return LineFunc(
        51411,
        "virtual int Dispatcher_JoinPowerWar::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233B32
// IDA_NAME: _ZN23Dispatcher_JoinPowerWar4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_JoinPowerWar::read(PacketBuf &,MSG_BASE &)
int Dispatcher_JoinPowerWar::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B3C
// IDA_NAME: _ZN23Dispatcher_JoinPowerWar7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_JoinPowerWar::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_JoinPowerWar::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
