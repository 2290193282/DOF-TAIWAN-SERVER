#include "old_game/player/core/packet_dispatcher_exit.h"

#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/security/security_card.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/db/account/db_arrange_user_login_info.h"
#include "old_data/db/anti_cheat/db_save_cleanpad_point.h"
#include "old_data/db/auth_security/db_secu_service_update_validity_time.h"
#include "old_data/db/character/db_update_charac_play_time.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

#include <string>


// IDA_EA: 0x859544C
// IDA_NAME: _ZN15DisPatcher_ExitC2Ev
// DEMANGLED: DisPatcher_Exit::DisPatcher_Exit(void)
DisPatcher_Exit::DisPatcher_Exit()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8232BF4
// IDA_NAME: _ZN15DisPatcher_ExitD2Ev
// DEMANGLED: DisPatcher_Exit::~DisPatcher_Exit()
// IDA_EA: 0x8232C24
// IDA_NAME: _ZN15DisPatcher_ExitD0Ev
// DEMANGLED: DisPatcher_Exit::~DisPatcher_Exit()
DisPatcher_Exit::~DisPatcher_Exit() {}

// IDA_EA: 0x81ED948
// IDA_NAME: _ZN15DisPatcher_Exit12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_Exit::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_Exit::dispatch_sig(CUser* user, PacketBuf& packet) {
    char peer_ip[16] = {};
    user->network_.GetPeerIP2(peer_ip, sizeof(peer_ip));

    unsigned char login_out_state = 0;
    if (packet.get_byte(login_out_state) != 1) {
        login_out_state = 0;
    }
    user->SetLogInOutState(login_out_state);

    WongWork::CMCAPManager& mcap_manager = user->getMCAPManagerW();
    if (mcap_manager.IsSaveCleanPadPoint()) {
        DB_SaveCleanpadPoint::makeRequest(mcap_manager.getPoint(),
                                          user->get_acc_id());
        mcap_manager.disableCleanPadPoint();
    }

    if (user->get_state() > 2 && G_GameWorld()->IsEquipSlotSwitchChannel() &&
        user->getCurChannelEquipslotSwitch() == 1) {
        CExpandEquipslot* const expand_equipslot =
            static_cast<CExpandEquipslot*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        if (expand_equipslot) {
            expand_equipslot->EquipslotSwitch(
                user,
                0,
                static_cast<char>(user->getCurChannelEquipslotSwitch()),
                static_cast<char>(user->getCurExpandEquipslotSwitch()));
        }
    }

    std::string event_manager_name("Arad_EventPeriodDataManager");
    ARAD::Arad_EventPeriodDataManager* const event_period_manager =
        static_cast<ARAD::Arad_EventPeriodDataManager*>(
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()->findGameScript(
                event_manager_name));
    event_period_manager->delUserRewardData(user);

    if (!user->get_state()) {
        return 0;
    }

    user->WorkPerFiveMin();
    if (user->get_charac_memberkey()) {
        const int money_tax = user->get_member_pay_tex_money_to_upper();
        const int fatigue_tax = user->get_member_pay_tex_fatigue_to_upper();
        CMonitorServerProxy* const monitor_proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        monitor_proxy->SendMemberPayTax(
            user->is_connect_upper_member() != 0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            money_tax,
            fatigue_tax);
        if (money_tax > 0) {
            user->set_member_pay_tex_money_to_upper(0);
        }
        if (fatigue_tax > 0) {
            user->set_member_pay_tex_fatigue_to_upper(0);
        }
    }

    cMyTrace trace("virtual int DisPatcher_Exit::dispatch_sig(CUser*, PacketBuf&)",
                   28781,
                   0);
    trace("[USER LOGOUT (Normal)] DB ID : %s\n",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));

    private_store::GetInstancePrivateStoreMgr()->Exit(user);
    pvp_assault::GetInstanceAssaultMgr()->OnLeaveAssaultPlace(user, true);
    GlobalData::s_ExpertJobMgr->OnLeaveUser(user);
    GlobalData::s_onlinePreliminaryTeamMgr->OnLeaveUser(user);
    user->checkLogOutCorrectly();

    const unsigned int account_id = static_cast<unsigned int>(user->get_acc_id());
    const unsigned int uid = static_cast<unsigned int>(user->GetUID());
    const bool security_card_certified = user->getSecurityCard()->isCertified();
    const bool pad_certified = user->getPad()->isCertified();
    DB_UpdateCharacPlayTime::makeRequest(user->get_charac_no(-1),
                                         user->GetCharacPlayTick());
    G_CGameManager()->user_exit(user);

    if (login_out_state) {
        DB_ArrangeUserLoginInfo::makeRequest(account_id, false, uid);
        if (security_card_certified) {
            DB_SecuServiceUpdateValidityTime::makeRequest(
                account_id, 180u, 8u, peer_ip);
        }
        if (pad_certified) {
            DB_SecuServiceUpdateValidityTime::makeRequest(
                account_id, 180u, 16u, peer_ip);
        }
    } else {
        DB_ArrangeUserLoginInfo::makeRequest(account_id, true, uid);
    }
    return 0;
}

// IDA_EA: 0x8232BE0
// IDA_NAME: _ZN15DisPatcher_Exit4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_Exit::read(PacketBuf &,MSG_BASE &)
int DisPatcher_Exit::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232BEA
// IDA_NAME: _ZN15DisPatcher_Exit7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_Exit::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_Exit::process(CUser* user,
                              MSG_BASE& message,
                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
