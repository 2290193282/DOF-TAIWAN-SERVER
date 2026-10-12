#include "old_data/Login/inter_returns/login_inter_receivers.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/account/db_arrange_user_login_info.h"
#include "old_data/db/anti_cheat/db_save_cleanpad_point.h"
#include "old_data/db/auth_security/db_security_card_update_validity_time.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

// These are original GlobalData storage symbols.  The public GlobalData
// façade intentionally does not retype them; their concrete ownership is
// restored by the matching expert-job and online-preliminary providers.

// IDA_EA: 0x84BEB3C
// IDA_NAME: _ZN16Inter_Disconnect12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Disconnect::dispatch_sig(CUser *,char *,int)
int Inter_Disconnect::dispatch_sig(CUser* user,
                                  char* /*payload*/,
                                  int /*payload_size*/) {
    if (!user->get_state()) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_Disconnect::dispatch_sig(CUser*, char*, int)",
            0x651,
            "State(NONE)");
        return 0;
    }

    if (user->get_state() > 2 && G_GameWorld()->IsEquipSlotSwitchChannel() &&
        user->getCurChannelEquipslotSwitch() == 1) {
        CExpandEquipslot* const expand_equipslot =
            static_cast<CExpandEquipslot*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        expand_equipslot->EquipslotSwitch(
            user,
            0,
            static_cast<char>(user->getCurChannelEquipslotSwitch()),
            static_cast<char>(user->getCurExpandEquipslotSwitch()));
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

    private_store::GetInstancePrivateStoreMgr()->Exit(user);

    // CUser::OnDisconnect @ 0x86804CE proves that zero is the documented
    // "not in an assault place" sentinel: it calls OnLeaveAssaultPlace only
    // after CUserCharacInfo::GetAssaultPlace() != 0.  Keep
    // CAssaultMgr::_GetAssaultPlace's original exception contract intact and
    // do not submit that sentinel to its map lookup from this early-login
    // disconnect path.
    const int assault_place_id = user->GetAssaultPlace();
    LogManager::logFormat(
        1,
        "disconnect_diag.cpp",
        "int Inter_Disconnect::dispatch_sig(CUser*, char*, int)",
        0,
        "[DIAG0627][INTER_DISCONNECT] uid=%d state=%d assault_place=%d",
        user->GetUID(), user->get_state(), assault_place_id);
    if (assault_place_id != 0) {
        pvp_assault::GetInstanceAssaultMgr()->OnLeaveAssaultPlace(user, true);
    }
    GlobalData::s_ExpertJobMgr->OnLeaveUser(user);
    GlobalData::s_onlinePreliminaryTeamMgr->OnLeaveUser(user);

    WongWork::CMCAPManager& mcap_manager = user->getMCAPManagerW();
    const unsigned int account_id = static_cast<unsigned int>(user->get_acc_id());
    const unsigned int user_id = static_cast<unsigned int>(user->GetUID());
    if (mcap_manager.IsSaveCleanPadPoint()) {
        DB_SaveCleanpadPoint::makeRequest(mcap_manager.getPoint(), account_id);
        mcap_manager.disableCleanPadPoint();
    }

    G_CGameManager()->user_disconnect(user);
    DB_ArrangeUserLoginInfo::makeRequest(account_id, true, user_id);
    DB_SecurityCardUpdateValidityTime::makeRequest(account_id, 0);
    DB_GoblinPadUpdateValidityTime::makeRequest(account_id, 0);
    return 0;
}

// IDA_EA: 0x84E9B0E
// IDA_NAME: _ZN16Inter_DisconnectC2Ev
// DEMANGLED: Inter_Disconnect::Inter_Disconnect(void)
Inter_Disconnect::Inter_Disconnect() : ISigDispatcher() {}

// IDA_EA: 0x84E9B2A / 0x84E9B5A
// IDA_NAME: _ZN16Inter_DisconnectD2Ev / _ZN16Inter_DisconnectD0Ev
// DEMANGLED: Inter_Disconnect::~Inter_Disconnect()
Inter_Disconnect::~Inter_Disconnect() {}
