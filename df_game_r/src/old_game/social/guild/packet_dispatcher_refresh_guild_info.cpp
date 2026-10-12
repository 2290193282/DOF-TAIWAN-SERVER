#include "old_game/social/guild/packet_dispatcher_refresh_guild_info.h"

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x81D487C
// IDA_NAME: _ZN27Dispatcher_RefreshGuildInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RefreshGuildInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RefreshGuildInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D4886
// IDA_NAME: _ZN27Dispatcher_RefreshGuildInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RefreshGuildInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RefreshGuildInfo::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)message;
    (void)param;

    if (user->get_state() <= 2) {
        return 0;
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (charac_guildkey) {
        const unsigned int charac_no =
            static_cast<unsigned int>(user->get_charac_no(-1));
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendRefreshGuildInfo(charac_no, charac_guildkey);
    }
    return 0;
}

// IDA_EA: 0x82452E0
// IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoD2Ev
// DEMANGLED: Dispatcher_RefreshGuildInfo::~Dispatcher_RefreshGuildInfo()
// IDA_EA: 0x8245310
// IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoD0Ev
// DEMANGLED: Dispatcher_RefreshGuildInfo::~Dispatcher_RefreshGuildInfo()
Dispatcher_RefreshGuildInfo::~Dispatcher_RefreshGuildInfo() {}

// IDA_EA: 0x8597442
// IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoC2Ev
// DEMANGLED: Dispatcher_RefreshGuildInfo::Dispatcher_RefreshGuildInfo(void)
Dispatcher_RefreshGuildInfo::Dispatcher_RefreshGuildInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>() {}
