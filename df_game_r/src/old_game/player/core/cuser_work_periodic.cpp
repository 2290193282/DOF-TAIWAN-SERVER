#include <stdint.h>
#include "common/manager/global_tools.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketHeader.h"

// IDA_EA: 0x8652F0C
// IDA_NAME: _ZN5CUser14WorkPerFiveMinEv
// DEMANGLED: CUser::WorkPerFiveMin(void)
void CUser::WorkPerFiveMin() {
    CUser* const user = this;
    int state = static_cast<int>(user->state_);
    if (state <= 2) {
        return;
    }

    if (!user->getCurCharacR()) {
        cMyTrace trace("void CUser::WorkPerFiveMin()", 7460, 5);
        trace(
            "CUser::WorkPerFiveMin() m_id(%s) character - m_selected is null",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0));
        return;
    }

    int guild_exp = user->get_guild_exp();
    if (guild_exp) {
        Packet_Monitor_Increase_Guild_Exp packet;
        packet.charac_no = static_cast<uint32_t>(getCurCharacNo());
        packet.charac_guildkey =
            static_cast<uint32_t>(get_charac_guildkey());
        packet.gained_exp = static_cast<uint32_t>(guild_exp);
        packet.source_flag = 0;
        packet.notify = guild_add_exp_flag_;
        history_log_.GuildAddExp(
            static_cast<unsigned int>(get_charac_guildkey()),
            reinterpret_cast<const char*>(GetGuildDBInfo()),
            static_cast<unsigned int>(getCurCharacNo()),
            guild_exp);
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
        proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
        guild_add_exp_flag_ = 0;
        user->reset_guild_exp();
    }

    state = user->get_guild_today_exp() != 0;
    if (static_cast<unsigned char>(state)) {
        const int today_exp = user->get_guild_today_exp();
        Packet_Monitor_Increase_Guild_Exp packet;
        packet.charac_no = static_cast<uint32_t>(getCurCharacNo());
        packet.charac_guildkey =
            static_cast<uint32_t>(get_charac_guildkey());
        packet.gained_exp = static_cast<uint32_t>(today_exp);
        packet.source_flag = 2;
        packet.notify = guild_add_exp_flag_;
        history_log_.GuildAddExp(
            static_cast<unsigned int>(get_charac_guildkey()),
            reinterpret_cast<const char*>(GetGuildDBInfo()),
            static_cast<unsigned int>(getCurCharacNo()),
            today_exp);
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
        proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
        guild_add_exp_flag_ = 0;
        state = user->reset_guild_today_exp();
    }
}

// IDA_EA: 0x8653196
// IDA_NAME: _ZN5CUser13WorkPerTenMinEv
// DEMANGLED: CUser::WorkPerTenMin(void)
char CUser::WorkPerTenMin() {
    const int state = get_state();
    if (state <= 2) {
        return static_cast<char>(state);
    }

    if (!getCurCharacR()) {
        cMyTrace trace("void CUser::WorkPerTenMin()", 7537, 5);
        return static_cast<char>(trace(
            "CUser::WorkPerTenMin() m_id(%s) character - m_selected is null",
            NumberToString(static_cast<unsigned int>(get_acc_id()), 0)));
    }

    if (!get_charac_memberkey()) {
        return static_cast<char>(state);
    }

    CMonitorServerProxy* const monitor_proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    return static_cast<char>(monitor_proxy->SendNoticeCharLive(
        static_cast<uint32_t>(getCurCharacNo())));
}
