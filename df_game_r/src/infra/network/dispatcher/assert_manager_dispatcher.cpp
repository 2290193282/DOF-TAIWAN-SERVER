#include "infra/network/dispatcher/assert_manager_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_AssertManager::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x822BD18
// IDA_NAME: _ZN26Packet_Assert_Manager_InfoC2Ev
// DEMANGLED: Packet_Assert_Manager_Info::Packet_Assert_Manager_Info(void)
Packet_Assert_Manager_Info::Packet_Assert_Manager_Info()
    : header(0x0C41u, 0x0214u) {}

// IDA_EA: 0x8596A80
// IDA_NAME: _ZN24Dispatcher_AssertManagerC2Ev
// DEMANGLED: Dispatcher_AssertManager::Dispatcher_AssertManager(void)
Dispatcher_AssertManager::Dispatcher_AssertManager() {}

// IDA_EA: 0x8241380
// IDA_NAME: _ZN24Dispatcher_AssertManagerD2Ev
// DEMANGLED: Dispatcher_AssertManager::~Dispatcher_AssertManager()
// IDA_EA: 0x82413B0
// IDA_NAME: _ZN24Dispatcher_AssertManagerD0Ev
// DEMANGLED: Dispatcher_AssertManager::~Dispatcher_AssertManager()
Dispatcher_AssertManager::~Dispatcher_AssertManager() {}

// IDA_EA: 0x821E1F4
// IDA_NAME: _ZN24Dispatcher_AssertManager12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AssertManager::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AssertManager::dispatch_sig(CUser* user, PacketBuf& packet) {
    Packet_Assert_Manager_Info statistic;

    if (packet.get_int(statistic.raw_0a) != 1) {
        return LineFunc(53877, kDispatchSignature, 0, 0);
    }
    if (statistic.raw_0a > 255) {
        return LineFunc(53881, kDispatchSignature, 0, 0);
    }
    if (packet.get_binary(statistic.raw_0e, statistic.raw_0a) != 1) {
        return LineFunc(53882, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.word_10e) != 1) {
        return LineFunc(53885, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(statistic.raw_110) != 1) {
        return LineFunc(53887, kDispatchSignature, 0, 0);
    }
    if (static_cast<unsigned int>(statistic.raw_110) >= 0x100u) {
        return LineFunc(53888, kDispatchSignature, 0, 0);
    }

    if (statistic.raw_110 <= 0) {
        std::memset(statistic.raw_114, 0, sizeof(statistic.raw_114));
    } else if (packet.get_binary(statistic.raw_114, statistic.raw_110) != 1) {
        return LineFunc(53891, kDispatchSignature, 0, 0);
    }

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x214);
    return 0;
}

// IDA_EA: 0x8233E3E
// IDA_NAME: _ZN24Dispatcher_AssertManager4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AssertManager::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AssertManager::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E48
// IDA_NAME: _ZN24Dispatcher_AssertManager7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AssertManager::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AssertManager::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
