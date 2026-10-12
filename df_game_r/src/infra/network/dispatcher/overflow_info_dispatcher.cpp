#include "infra/network/dispatcher/overflow_info_dispatcher.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_Overflow_Info::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x822BDF0
// IDA_NAME: _ZN29Packet_Overflow_Statistic_AddC2Ev
// DEMANGLED: Packet_Overflow_Statistic_Add::Packet_Overflow_Statistic_Add(void)
Packet_Overflow_Statistic_Add::Packet_Overflow_Statistic_Add()
    : header(0x0C43u, 0x000Du) {}

// IDA_EA: 0x8596AB8
// IDA_NAME: _ZN24Dispatcher_Overflow_InfoC2Ev
// DEMANGLED: Dispatcher_Overflow_Info::Dispatcher_Overflow_Info(void)
Dispatcher_Overflow_Info::Dispatcher_Overflow_Info() {}

// IDA_EA: 0x82412E4
// IDA_NAME: _ZN24Dispatcher_Overflow_InfoD2Ev
// DEMANGLED: Dispatcher_Overflow_Info::~Dispatcher_Overflow_Info()
// IDA_EA: 0x8241314
// IDA_NAME: _ZN24Dispatcher_Overflow_InfoD0Ev
// DEMANGLED: Dispatcher_Overflow_Info::~Dispatcher_Overflow_Info()
Dispatcher_Overflow_Info::~Dispatcher_Overflow_Info() {}

// IDA_EA: 0x821E4B6
// IDA_NAME: _ZN24Dispatcher_Overflow_Info12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Overflow_Info::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Overflow_Info::dispatch_sig(CUser* user, PacketBuf& packet) {
    Packet_Overflow_Statistic_Add statistic;
    if (packet.get_byte(statistic.raw_0a) != 1) {
        return LineFunc(54575, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_0b) != 1) {
        return LineFunc(54576, kDispatchSignature, 0, 0);
    }

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x0D);
    return 0;
}

// IDA_EA: 0x8233E66
// IDA_NAME: _ZN24Dispatcher_Overflow_Info4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Overflow_Info::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Overflow_Info::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E70
// IDA_NAME: _ZN24Dispatcher_Overflow_Info7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Overflow_Info::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Overflow_Info::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
