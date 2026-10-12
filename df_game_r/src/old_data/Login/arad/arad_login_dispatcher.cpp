#include "old_data/Login/arad/arad_login_dispatcher.h"

#include "core/config/env/environment.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/crypto/global_data_legacy_crypto.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/arad/login_punish.h"
#include "old_data/Login/simple_sso.h"
#include "old_data/db/account/arad_login_query.h"
#include "old_data/db/core/db_mgr.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "common/manager/system_time.h"
#include "infra/security/crypto/arad_crypt.h"

#include <ctime>
#include <cstring>
#include <stdint.h>
#include <vector>

namespace ARAD {

// IDA_EA: 0x818EED0
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_ImplC2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::Arad_Dispatcher_Login_Impl()
Arad_Dispatcher_Login_Impl::Arad_Dispatcher_Login_Impl()
    : Arad_Dispatcher(1) {
}

// IDA_EA: 0x818EEF4
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_ImplD2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::~Arad_Dispatcher_Login_Impl()
// IDA_EA: 0x818EF24
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_ImplD0Ev
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::~Arad_Dispatcher_Login_Impl()
Arad_Dispatcher_Login_Impl::~Arad_Dispatcher_Login_Impl() {
}

// IDA_EA: 0x818EF42
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl14inter_dispatchEP5CUserPci
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::inter_dispatch(CUser *,char *,int)
int Arad_Dispatcher_Login_Impl::inter_dispatch(CUser*, char*, int) {
    return 0;
}

// IDA_EA: 0x818EF4C
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl11db_dispatchEiiP6Stream
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::db_dispatch(int,int,Stream *)
int Arad_Dispatcher_Login_Impl::db_dispatch(int, int user_idx, Stream* stream) {
    SIG_LOGIN_DATA* const login = stream->GetOutBuffer<SIG_LOGIN_DATA>();
    if (!login) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatcher_Login_Impl.cpp",
            "virtual int ARAD::Arad_Dispatcher_Login_Impl::db_dispatch(int, int, Stream*)",
            0x39,
            "%s",
            "recv stream buffer null!!");
        return -1;
    }

    login->fail_code = 0;
    if (G_GameWorld()->GetChannelType() == 8) {
        DBMgr::stUserDBInfo_t user_db_info = {0, 0, 0};
        user_db_info.server_id = static_cast<uint32_t>(
            static_cast<int32_t>(login->server_group));
        GlobalData::s_db_mgr->addUserDBInfo(user_idx, user_db_info);
        GlobalData::s_db_mgr->setUserIdx(user_idx);
    }

    char password[132] = {};
    if (DATABASE::LOGIN_QUERY::arad_sp_select_account_info(login, password)) {
        login->fail_code = 4;
    }
    if (account_password_check(login, password)) {
        login->fail_code = 1;
    }
    if (login->account_fail > 2 || login->total_account_fail > 9) {
        login->fail_code = 7;
    }
    if (DATABASE::LOGIN_QUERY::arad_sp_select_punish_user_info(login)) {
        login->fail_code = 15;
    }
    DATABASE::LOGIN_QUERY::arad_sp_select_punish_second_user(login);
    punish_user_check(login);
    DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(login);
    pad_certifiaction_check(login);

    std::vector<DB_Login::tagDNFEventEntry> event_entries;
    DATABASE::LOGIN_QUERY::arad_sp_load_event_entry(login, event_entries);
    premium_info_set(login, event_entries, user_idx);

    if (G_GameWorld()->GetChannelType() == 7 &&
        (login->current_charac_no == 0 ||
         login->school_broadcast_server_group == 0)) {
        login->fail_code = 8;
    }
    return 0;
}

// IDA_EA: 0x818F1CA
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl16premium_info_setEP14SIG_LOGIN_DATARSt6vectorIN8DB_Login16tagDNFEventEntryESaIS5_EEi
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::premium_info_set(SIG_LOGIN_DATA *,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &,int)
int Arad_Dispatcher_Login_Impl::premium_info_set(
    SIG_LOGIN_DATA* login,
    std::vector<DB_Login::tagDNFEventEntry>& event_entries,
    int user_idx) {
    int premium_count = 0;
    login->premium_count = 0;
    login->premium_event_count = 0;
    login->daily_clean_pad_point = 0;

    std::vector<int> event_ids;
    std::vector<int> empty_event_ids;
    for (std::vector<DB_Login::tagDNFEventEntry>::iterator it =
             event_entries.begin();
         it != event_entries.end();
         ++it) {
        DB_Login::tagDNFEventEntry event_entry = *it;
        if (event_entry.event_id > 49999 && event_entry.event_id <= 89999) {
            const bool special_event =
                event_entry.event_id > 69999 && event_entry.event_id <= 89999;
            if (special_event ||
                WongWork::CUserPremium::IsRestrictedPremium(
                    event_entry.event_id)) {
                const uint16_t index = login->premium_event_count;
                login->premium_event_ids[index] = event_entry.event_id;
                login->premium_event_count =
                    static_cast<uint16_t>(index + 1);
            } else if (event_entry.event_id > 41000 &&
                       event_entry.event_id <= 41003) {
                DATABASE::LOGIN_QUERY::arad_sp_select_member_brodcast(
                    login, event_entry, user_idx);
            } else {
                std::vector<int> remove_event_ids(empty_event_ids);
                DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(
                    login, event_entry, event_ids, remove_event_ids);
            }
        }
    }

    login->premium_count = premium_count;
    return 0;
}

// IDA_EA: 0x818F4A6
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl23pad_certifiaction_checkEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::pad_certifiaction_check(SIG_LOGIN_DATA *)
int Arad_Dispatcher_Login_Impl::pad_certifiaction_check(SIG_LOGIN_DATA* login) {
    if (login->passpad_result == 2) {
        char goblin_time[20] = {};
        const std::time_t first_time = static_cast<std::time_t>(
            GlobalData::s_systemTime_.getCurSec());
        std::tm local_time;
        localtime_r(&first_time, &local_time);
        const int day_offset = local_time.tm_hour <= 5 ? 86400 : 0;
        const std::time_t check_time = static_cast<std::time_t>(
            GlobalData::s_systemTime_.getCurSec()) - day_offset;
        localtime_r(&check_time, &local_time);
        std::strftime(goblin_time, sizeof(goblin_time),
                      "%Y-%m-%d 06:00:00", &local_time);

        cMyTrace trace(
            "static int ARAD::Arad_Dispatcher_Login_Impl::pad_certifiaction_check(SIG_LOGIN_DATA*)",
            282,
            0);
        trace("[Goblin Check Time(%s)]]", goblin_time);
        return DATABASE::LOGIN_QUERY::arad_sp_check_goblin_reward(login,
                                                                    goblin_time);
    }

    login->passpad_result = 1;
    char source[0x24] = {};
    char encrypted[0x24] = {};
    std::strncpy(source, login->encrypted_password, sizeof(source));
    ARAD_CRYPT::encryptAES(
        GlobalData::s_pcryptRijndael_MousePassword_, source, encrypted);
    if (std::strncmp(login->passpad_mousepass, encrypted, sizeof(encrypted))) {
        ++login->passpad_fail_count;
        DATABASE::LOGIN_QUERY::arad_sp_update_fail_record(login);
        if (login->passpad_fail_count <= 9) {
            login->fail_code = 5;
            return 72;
        }
        login->fail_code = 6;
        return 73;
    }

    login->passpad_fail_count = 0;
    login->passpad_cancel_count = 0;
    if (DATABASE::LOGIN_QUERY::arad_sp_update_history(login, 7)) {
        login->fail_code = 6;
        return 1;
    }
    if (DATABASE::LOGIN_QUERY::arad_sp_delete_member_mouspass(login)) {
        login->fail_code = 6;
        return 1;
    }
    login->passpad_enabled = false;
    return 0;
}

// IDA_EA: 0x818F746
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl22account_password_checkEP14SIG_LOGIN_DATAPc
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::account_password_check(SIG_LOGIN_DATA *,char *)
int Arad_Dispatcher_Login_Impl::account_password_check(SIG_LOGIN_DATA* login,
                                                        char* password) {
    (void)std::time(0);
    long login_time = 0;
    long expire_time = 0;
    WongWork::CSimpleSSO::stOutputData_t output;
    if (GlobalData::s_psimpleSSO->getGameStartTime(
            login->member_id, login_time, expire_time, output) != 1) {
        cMyTrace trace(
            "int ARAD::Arad_Dispatcher_Login_Impl::account_password_check(SIG_LOGIN_DATA*, char*)",
            362,
            5);
        trace("get game start time error : user_idx[%d]", login->member_id);
        return -1;
    }

    if (GlobalData::s_GM_Manager->IsGm(login->member_id)) {
        if (std::strncmp(password, login->login_password, 0x84) == 0) {
            return 0;
        }
        cMyTrace trace(
            "int ARAD::Arad_Dispatcher_Login_Impl::account_password_check(SIG_LOGIN_DATA*, char*)",
            374,
            5);
        trace("gm user password error user_id[%d]", login->member_id);
        return -1;
    }

    login->total_account_fail = output.total_account_fail;
    login->report_count = output.report_count;
    login->account_fail = output.account_fail;
    login->last_play_time = output.last_play_time;
    login->trade_gold_daily = output.trade_gold_daily;
    login->reliable_flag = output.reliable_flag;
    login->tutorial_skipable = output.tutorial_skipable;
    login->security_card_service = output.security_flag;
    login->dungeon_gain_gold = output.dungeon_gain_gold;
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               output.last_play_time,
                               GlobalData::s_systemTime_.getCurSec())) {
        login->dungeon_gain_gold = 0;
    }
    std::strncpy(login->sso_login_ip, output.login_ip,
                 sizeof(login->sso_login_ip));
    return 0;
}

// IDA_EA: 0x818F970
// IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl17punish_user_checkEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::punish_user_check(SIG_LOGIN_DATA *)
int Arad_Dispatcher_Login_Impl::punish_user_check(SIG_LOGIN_DATA* login) {
    const int32_t now = GlobalData::s_systemTime_.getCurSec();
    for (int i = 0; static_cast<int32_t>(login->punish_count) > i; ++i) {
        const int32_t end_time =
            static_cast<int32_t>(login->punish_rows[i].end_time);
        const int punish_type = login->punish_rows[i].punish_type;
        PUNISH_CHECK::punish_check_by_HUB(login, punish_type);
        PUNISH_CHECK::punish_check_by_RES(login, punish_type);
        if (now <= end_time) {
            if (!PUNISH_CHECK::punish_check_by_WEB(login, end_time,
                                                    punish_type) ||
                !PUNISH_CHECK::punish_check_by_SYS(login, end_time,
                                                    punish_type) ||
                !PUNISH_CHECK::punish_check_by_ARS(login, end_time,
                                                    punish_type)) {
                login->punish_count = static_cast<uint32_t>(i);
                return 0;
            }
        }
    }
    return 1;
}

}  // namespace ARAD
