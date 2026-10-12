#include "old_game/world/region/game_world.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/power_parameter_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <vector>

// IDA_EA: 0x86CEDA0
// IDA_NAME: _ZN9GameWorld13OnEndPowerWarER11PacketGuard
// DEMANGLED: GameWorld::OnEndPowerWar(PacketGuard &)
int GameWorld::OnEndPowerWar(PacketGuard& packet) {
    packet.put_byte(GlobalData::s_power_manager->GetWinnerSide());
    return packet.put_short(0);
}

// IDA_EA: 0x86CEDF4
// IDA_NAME: _ZN9GameWorld28OnRefreshPowerWarProcessInfoEv
// DEMANGLED: GameWorld::OnRefreshPowerWarProcessInfo(void)
bool GameWorld::OnRefreshPowerWarProcessInfo() {
    GameWorldUserMap& users = users_in_world_;
    if (users.empty()) {
        return false;
    }

    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        CUser* const user = it->second;
        if (!user->isJoinPowerWar()) {
            continue;
        }

        const int current_second = GlobalData::system_time()->getCurSec();
        const int refresh_seconds = 60 *
            G_CDataManager()->get_power_war_parameter_script()->value_30;
        if (current_second - user->GetPowerWarProcessInfoLastUpdateTime() >
            refresh_seconds) {
            CGuildServerProxy* const guild_proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            guild_proxy->SendPowerWarProcessInfo(
                static_cast<uint32_t>(user->get_charac_no(-1)));
        }
    }

    // The original loop returns the final iterator comparison, which is
    // false after a complete traversal rather than a "did send" indicator.
    return false;
}

// IDA_EA: 0x86CEF36
// IDA_NAME: _ZN9GameWorld23OnBanAbuserFromPowerWarEv
// DEMANGLED: GameWorld::OnBanAbuserFromPowerWar(void)
bool GameWorld::OnBanAbuserFromPowerWar() {
    GameWorldUserMap& users = users_in_world_;
    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        CUser* const user = it->second;
        if (!user->isJoinPowerWar()) {
            continue;
        }

        const int current_second = GlobalData::s_systemTime_.getCurSec();
        const int last_play_second = user->GetPowerWarLastPlayTime();
        const int ban_seconds = 60 *
            G_CDataManager()->get_power_war_parameter_script()->value_34;
        if (current_second - last_play_second <= ban_seconds) {
            continue;
        }

        GlobalData::s_power_manager->ProcessJoinPowerWar(user, 0);

        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_POWER_WAR_BAN_ABUSER);
        packet.finalize(true);
        user->Send(packet);

        cMyTrace trace("void GameWorld::OnBanAbuserFromPowerWar()", 6030, 0);
        trace("[PowerWar] POWER WAR BAN USER (USER:%s(%d), LAST:%d, CURR:%d)",
              user->getCurCharacName(), user->getCurCharacNo(),
              last_play_second, current_second);
    }

    // The original returns the final map-iterator comparison, hence false
    // after both an empty and a fully traversed directory.
    return false;
}

// IDA_EA: 0x86CF138
// IDA_NAME: _ZN9GameWorld26CleanupPowerWarVillToStartEv
// DEMANGLED: GameWorld::CleanupPowerWarVillToStart(void)
bool GameWorld::CleanupPowerWarVillToStart() {
    GameWorldUserMap& users = users_in_world_;
    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        CUser* const user = it->second;
        if (static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
            GlobalData::s_power_manager->ProcessJoinPowerWar(user, 0);
        }
    }

    // Matches the original terminal iterator comparison.
    return false;
}

// IDA_EA: 0x86CF1FE
// IDA_NAME: _ZN9GameWorld16GetPowerUpDomainERSt6vectorIP5CUserSaIS2_EES5_
// DEMANGLED: GameWorld::GetPowerUpDomain(std::vector<CUser *,std::allocator<CUser *> > &,std::vector<CUser *,std::allocator<CUser *> > &)
bool GameWorld::GetPowerUpDomain(std::vector<CUser*>& power_a_users,
                                 std::vector<CUser*>& power_b_users) {
    const double failure_ratio_threshold =
        static_cast<double>(
            G_CDataManager()->get_power_war_parameter_script()->value_50) /
        100.0;

    GameWorldUserMap& users = users_in_world_;
    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        CUser* const user = it->second;
        if (!user->isJoinPowerWar() || user->GetPowerWarAssaultCount() == 0) {
            continue;
        }

        const double assault_count =
            static_cast<double>(user->GetPowerWarAssaultCount());
        const double victory_count =
            static_cast<double>(user->GetPowerWarAssaultVictoryCount());
        if (failure_ratio_threshold >= victory_count / assault_count) {
            if (user->getPowerSide() == 1) {
                power_a_users.push_back(user);
            } else {
                power_b_users.push_back(user);
            }
        }
    }
    return false;
}

// IDA_EA: 0x86CF356
// IDA_NAME: _ZN9GameWorld13CancelPowerUpEv
// DEMANGLED: GameWorld::CancelPowerUp(void)
void GameWorld::CancelPowerUp() {
    std::vector<CUser*> power_up_users;
    GameWorldUserMap& users = users_in_world_;
    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        CUser* const user = it->second;
        if (user->IsPowerUp()) {
            user->SetPowerUp(false);
            power_up_users.push_back(user);
        }
    }

    SendPowerUpList(static_cast<ENUM_POWER_UP_ACTIVATE_TYPE>(0),
                    power_up_users);
}

// IDA_EA: 0x86CF46A
// IDA_NAME: _ZN9GameWorld15SendPowerUpListE27ENUM_POWER_UP_ACTIVATE_TYPERSt6vectorIP5CUserSaIS3_EE
// DEMANGLED: GameWorld::SendPowerUpList(ENUM_POWER_UP_ACTIVATE_TYPE,std::vector<CUser *,std::allocator<CUser *> > &)
void GameWorld::SendPowerUpList(ENUM_POWER_UP_ACTIVATE_TYPE activate_type,
                                std::vector<CUser*>& users) {
    PacketGuard packet;
    packet.put_header(0, 216);
    packet.put_byte(static_cast<int>(activate_type));
    packet.put_short(static_cast<int>(users.size()));
    for (std::vector<CUser*>::iterator it = users.begin(); it != users.end();
         ++it) {
        packet.put_short((*it)->get_unique_id());
    }
    packet.finalize(true);
    G_GameWorld()->send_to_vill(7, packet);
}
