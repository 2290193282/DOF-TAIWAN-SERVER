#include "old_data/Login/packet_dispatch/power_war_process_info_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kPowerWarProcessInfoSignature[] =
    "virtual int Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8596B28
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoC2Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::Dispatcher_PowerWarProcessInfo(void)
Dispatcher_PowerWarProcessInfo::Dispatcher_PowerWarProcessInfo() {}

// IDA_EA: 0x82411FA
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoD2Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo()
// IDA_EA: 0x824122A
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoD0Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo()
Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo() {}

// IDA_EA: 0x821EA32
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(55073, kPowerWarProcessInfoSignature, 0, 0);
    }

    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendPowerWarProcessInfo(charac_no);

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_POWER_WAR_PROCESS_INFO);
    response.put_byte(1);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233EB6
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PowerWarProcessInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PowerWarProcessInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233EC0
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PowerWarProcessInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PowerWarProcessInfo::process(CUser* user, MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
