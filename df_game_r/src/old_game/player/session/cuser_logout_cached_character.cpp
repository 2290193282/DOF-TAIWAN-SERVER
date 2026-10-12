#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/double_conn_check_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/user.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/player/core/cuser.h"

#include <stdint.h>



// IDA_EA: 0x864CB66
// IDA_NAME: _ZN5CUser21LogoutCachedCharacterEh
// DEMANGLED: CUser::LogoutCachedCharacter(unsigned char)
int CUser::LogoutCachedCharacter(unsigned char flag) {
    CUser* const user = this;
    user->SendMoenyLog();
    const unsigned char cached = static_cast<unsigned char>(
        user->CacheCharacterMemory());

    const unsigned int member_key = static_cast<unsigned int>(
        user->get_charac_memberkey());
    const unsigned int guild_key = static_cast<unsigned int>(
        user->get_charac_guildkey());
    const unsigned int charac_no = static_cast<unsigned int>(
        user->get_charac_no(-1));
    char* const charac_name = const_cast<char*>(
        user->getCurCharacName());
    const int channel_no = G_CEnvironment()->get_channel_no();
    const unsigned int account_id = static_cast<unsigned int>(
        user->get_acc_id());
    const int server_group = user->GetServerGroup();
    GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(server_group))
        ->SendLogout(account_id,
                     channel_no,
                     charac_name,
                     charac_no,
                     guild_key,
                     member_key,
                     flag);

    const unsigned int monitor_member_key = static_cast<unsigned int>(
        user->get_charac_memberkey());
    const unsigned int monitor_guild_key = static_cast<unsigned int>(
        user->get_charac_guildkey());
    const unsigned int monitor_charac_no = static_cast<unsigned int>(
        user->get_charac_no(-1));
    char* const monitor_charac_name = const_cast<char*>(
        user->getCurCharacName());
    const int monitor_channel_no = G_CEnvironment()->get_channel_no();
    const unsigned int monitor_account_id = static_cast<unsigned int>(
        user->get_acc_id());
    const int monitor_server_group = user->GetServerGroup();
    GlobalData::s_monitor_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(monitor_server_group))
        ->SendLogout(
        monitor_account_id,
        monitor_channel_no,
        monitor_charac_name,
        monitor_charac_no,
        monitor_guild_key,
        monitor_member_key,
        flag,
        cached);

    if (!flag) {
        const int server_no = G_CEnvironment()->get_server_group();
        reinterpret_cast<CDoubleConnCheckServerProxy*>((GlobalData::s_double_check_proxy))->SendLogout(
            static_cast<unsigned int>(user->get_acc_id()),
            G_CEnvironment()->get_channel_no(),
            server_no);
    }

    if (user->getCurCharacR()) {
        user->SendLogoutToPCRoomServer();
    }
    user->delete_ontime_reward_list();
    ARAD::Singleton<HeroMissionEvent>::Get()->saveEventData(user, true);
    return 1;
}
