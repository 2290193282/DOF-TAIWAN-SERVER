#include "old_game/pvp/battlefield/power_war_runtime.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/power_parameter_script.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x847FA7C
// IDA_NAME: _ZN13CPowerManager23CheckPowerWarEnterLimitEP5CUser
// DEMANGLED: CPowerManager::CheckPowerWarEnterLimit(CUser *)
int CPowerManager::CheckPowerWarEnterLimit(CUser* user) {
    const PowerParameterScript* const parameters =
        G_CDataManager()->get_power_war_parameter_script();
    int minimum_level = parameters->value_64;
    int minimum_age = parameters->value_68;
    if (!minimum_level) {
        minimum_level = 30;
    }
    if (!minimum_age) {
        minimum_age = 15;
    }

    int ignored_unique_id = 0;
    if (user->CheckLimitMiniumAge(minimum_age, ignored_unique_id) != 1) {
        return 95;
    }
    return user->get_charac_level() >= minimum_level ? 0 : 14;
}

// IDA_EA: 0x847FB02
// IDA_NAME: _ZN13CPowerManager19ProcessJoinPowerWarEP5CUserc
// DEMANGLED: CPowerManager::ProcessJoinPowerWar(CUser *,char)
int CPowerManager::ProcessJoinPowerWar(CUser* user, char join) {
    if (user->get_charac_job() ==
            oldsrv::config::kCharacterJobDemonicSwordman ||
        user->get_charac_job() ==
            oldsrv::config::kCharacterJobCreatorMage ||
        user->isCompetitionMercenary()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 21);
        return 1;
    }
    if (!user->get_charac_guildkey()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 100);
        return 1;
    }
    if (!user->getPowerSide()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 130);
        return 1;
    }
    if (static_cast<unsigned char>(join) >= 2u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 1);
        return 1;
    }

    if (join == 1) {
        CEnvironment* const environment = G_CEnvironment();
        PowerParameterScript* const parameters =
            G_CDataManager()->get_power_war_parameter_script();
        if (parameters->checkPowerWarServer(environment->get_server_group(),
                                            environment->get_channel_no()) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 128);
            return 1;
        }
        if (!IsPowerWarEventOn()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 127);
            return 1;
        }
        const int enter_limit = CheckPowerWarEnterLimit(user);
        if (enter_limit) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR,
                                     static_cast<unsigned char>(enter_limit));
            return 1;
        }

        const int cooldown_seconds = 60 * parameters->value_b8;
        const int current_second = GlobalData::s_systemTime_.getCurSec();
        if (current_second - user->getCurCharacLastPlayTickPowerWar() <=
            cooldown_seconds) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 132);
            return 1;
        }

        if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(
                user) ||
            user->IsBusyDisjointStore()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_JOIN_POWER_WAR, 1);
            return 0;
        }
    }

    user->setCurrCharacJoinPower(join);
    const int power_side = user->getPowerSide();
    const bool temporary_power_side = power_side == 3 || power_side == 4;
    if (join == 1) {
        if (temporary_power_side) {
            GlobalData::s_guild_proxy_mgr
                ->GetServerProxy(SERVER_GROUP_DEFAULT)
                ->SendApplyOriginalPowerSide(
                    static_cast<unsigned int>(user->getCurCharacNo()),
                    static_cast<unsigned int>(user->get_charac_guildkey()),
                    static_cast<unsigned char>(power_side));
        }

        user->SetPowerWarLastPlayTime(GlobalData::s_systemTime_.getCurSec());
        user->setPowerWarHP(
            static_cast<uint16_t>(user->getCurCharacStamina()));
        user->resetStraightVictories();
        GlobalData::s_power_manager->IncreasePowerUserCount(
            static_cast<char>(user->getPowerSide()));
    } else {
        GlobalData::s_power_manager->DecreasePowerUserCount(
            static_cast<char>(user->getPowerSide()));
        user->setCurCharacLastPlayTickPowerWar(
            GlobalData::s_systemTime_.getCurSec());
        user->SetCurCharacChaosDieTime(0);
        user->character_view().enableSaveCharacView();
    }

    if (!temporary_power_side) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_JOIN_POWER_WAR);
        packet.put_byte(1);
        packet.put_byte(join);
        packet.finalize(true);
        user->Send(packet);

        PacketGuard basic_info;
        basic_info.put_header(0, 2);
        basic_info.put_byte(0);
        basic_info.put_short(1);
        user->make_basic_info(basic_info, 0);
        basic_info.finalize(true);
        G_GameWorld()->send_all(basic_info, user);
    }
    return 1;
}

// IDA_EA: 0x8480102
// IDA_NAME: _ZN13CPowerManager22IncreasePowerUserCountEc
// DEMANGLED: CPowerManager::IncreasePowerUserCount(char)
CPowerManager* CPowerManager::IncreasePowerUserCount(char power_side) {
    if (power_side == 1 || power_side == 3) {
        ++power_user_count_a_;
    } else {
        ++power_user_count_b_;
    }
    return this;
}
