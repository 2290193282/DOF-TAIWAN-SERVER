#include "old_data/db/account/arad_login_query.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"

#include <stddef.h>
#include <string.h>


namespace {

const ENUM_DB_HANDLE_IDX kPunishUserInfoDbHandle = DB_HANDLE_1;
const ENUM_DB_HANDLE_IDX kMemberMousePassDbHandle = DB_HANDLE_LOG_REF;
const ENUM_SERVER_GROUP kDefaultServerGroup = SERVER_GROUP_DEFAULT;

static_assert(offsetof(DB_Login::tagDNFEventEntry, event_id) == 0x00u,
               "Premium query event ID must stay at +0x00");
static_assert(sizeof(DB_Login::tagDNFEventEntry) == 0x18u,
              "Premium query event entry must keep the old 24-byte ABI");
static_assert(offsetof(DB_Login::tagDNFEventEntry, server_id) == 0x0cu,
               "Premium query server filter source must stay at +0x0C");
static_assert(offsetof(SIG_LOGIN_DATA, premium_entries) == 0x23b8u,
              "Premium entries must stay at +0x23B8");
static_assert(offsetof(SIG_LOGIN_DATA, premium_event_count) == 0x2a5au,
              "Premium event count must stay at +0x2A5A");
static_assert(offsetof(SIG_LOGIN_DATA, premium_event_ids) == 0x2a5cu,
              "Premium event IDs must stay at +0x2A5C");
static_assert(offsetof(SIG_LOGIN_DATA, member_id) == 0x00c0u,
              "Account-query member ID must stay at +0xC0");
static_assert(offsetof(SIG_LOGIN_DATA, birth_year) == 0x00c8u,
              "Account-query birth year must stay at +0xC8");
static_assert(offsetof(SIG_LOGIN_DATA, register_day) == 0x00ccu,
              "Account-query registration day must stay at +0xCC");
static_assert(offsetof(SIG_LOGIN_DATA, user_name) == 0x22a4u,
              "Account-query user name must stay at +0x22A4");
static_assert(offsetof(SIG_LOGIN_DATA, email) == 0x2324u,
              "Account-query email must stay at +0x2324");
static_assert(offsetof(SIG_LOGIN_DATA, member_identity) == 0x23a4u,
              "Account-query identity text must stay at +0x23A4");
static_assert(offsetof(SIG_LOGIN_DATA, hangame_flag) == 0x39fdu,
              "Account-query Hangame flag must stay at +0x39FD");
static_assert(offsetof(SIG_LOGIN_DATA, school_broadcast_server_group) ==
                  0x3ac8u,
              "Broadcast server group must stay at +0x3AC8");
static_assert(offsetof(SIG_LOGIN_DATA, current_charac_no) == 0x3accu,
              "Broadcast character number must stay at +0x3ACC");
static_assert(offsetof(SIG_LOGIN_DATA, current_charac_name) == 0x3ad0u,
              "Broadcast character name must stay at +0x3AD0");
static_assert(offsetof(SIG_LOGIN_DATA, punish_second_trade_count) == 0x3b44u,
              "Second-punish count must stay at +0x3B44");
static_assert(offsetof(SIG_LOGIN_DATA, punish_second_trade_gold) == 0x3b48u,
              "Second-punish trade gold must stay at +0x3B48");

}  // namespace


namespace ARAD {
namespace DATABASE {
namespace LOGIN_QUERY {

// IDA_EA: 0x818B706
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY27arad_sp_select_account_infoEP14SIG_LOGIN_DATAPc
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA *,char *)
int arad_sp_select_account_info(SIG_LOGIN_DATA* payload, char* password) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kPunishUserInfoDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x1e8,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect m_id, passwd,first_ssn, second_ssn, mid(from_unixtime(reg_date)+0,1,8),user_name,email, user_id, hangame_flag from member_info where m_id=%d",
            payload->member_id) != 0) {
        return -1;
    }

    if (db_handle->get_n_rows() != 1) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x1f5,
            5);
        trace("query get_n_rows() ERROR m_id=%u", payload->member_id);
        return 0;
    }

    unsigned int first_ssn = 0;
    unsigned int second_ssn = 0;
    unsigned int register_day = 0;
    unsigned int hangame_flag = 0;
    int column = 1;

    if (db_handle->get_uint(column, payload->member_id) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x201,
            5);
        trace("user m_id ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_str(++column, password, 132) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x207,
            5);
        trace("user password ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_uint(++column, first_ssn) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x20d,
            5);
        trace("user fssn ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_uint(++column, second_ssn) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x213,
            5);
        trace("user sssn ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_uint(++column, register_day) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x219,
            5);
        trace("user regdate ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_str(++column, payload->user_name, 128) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x21f,
            5);
        trace("user user_name ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_str(++column, payload->email, 128) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x225,
            5);
        trace("user email ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_str(++column, payload->login_user_id, 30) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x22b,
            5);
        trace("user id ERROR m_id=%u", payload->member_id);
        return 0;
    }
    if (db_handle->get_uint(++column, hangame_flag) != true) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_account_info(SIG_LOGIN_DATA*, char*)",
            0x231,
            5);
        trace("user HanGameUser Check ERROR m_id=%u", payload->member_id);
        return 0;
    }

    payload->register_day = register_day;
    payload->hangame_flag = hangame_flag == 1;
    OS_API::snprintf(payload->member_identity,
                     0x10u,
                     "%d%d",
                     first_ssn,
                     second_ssn);

    const unsigned int birth_year = first_ssn / 10000u;
    const unsigned int second_group = second_ssn / 1000000u;
    payload->birth_year = birth_year +
        ((second_group <= 2) ? 1900u : 2000u);
    return 0;
}

// IDA_EA: 0x818BFB0
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY33arad_sp_select_punish_second_userEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_punish_second_user(SIG_LOGIN_DATA *)
int arad_sp_select_punish_second_user(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_punish_second_user(SIG_LOGIN_DATA*)",
            0x293,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect trade_cnt,trade_gold,punish_flag,unix_timestamp(occ_time) from auto_punish_second_user where m_id=%d",
            payload->member_id) != 0) {
        return -1;
    }

    char punish_flag = 0;
    int occurrence_time = 0;
    db_handle->get_uint(0, payload->punish_second_trade_count);
    db_handle->get_ulonglong(1, payload->punish_second_trade_gold);
    db_handle->get_byte(2, punish_flag);
    db_handle->get_int(3, occurrence_time);
    return punish_flag ? 0 : -1;
}

// IDA_EA: 0x818BD67
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY31arad_sp_select_punish_user_infoEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_punish_user_info(SIG_LOGIN_DATA *)
int arad_sp_select_punish_user_info(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kPunishUserInfoDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_punish_user_info(SIG_LOGIN_DATA*)",
            0x255,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect punish_type, unix_timestamp(occ_time),punish_value, apply_flag, unix_timestamp(start_time),unix_timestamp(end_time) from member_punish_info where m_id=%d and apply_flag=1 LIMIT %d",
            static_cast<int>(payload->member_id),
            50) != 0) {
        return -1;
    }

    // The old leaf walks the selected rows directly (without fetch()) and
    // deliberately leaves payload->punish_count unchanged.
    for (unsigned int row_index = 0;
         static_cast<unsigned int>(db_handle->get_n_rows()) > row_index;
         ++row_index) {
        SIG_LOGIN_DATA::PunishRow& punish = payload->punish_rows[row_index];
        if (db_handle->get_int(0, punish.punish_type) != true) {
            return -1;
        }
        if (db_handle->get_uint(1, punish.occurrence_time) != true) {
            return -1;
        }
        if (db_handle->get_int(2, punish.punish_value) != true) {
            return -1;
        }
        if (db_handle->get_int(3, punish.apply_flag) != true) {
            return -1;
        }
        if (db_handle->get_uint(4, punish.start_time) != true) {
            return -1;
        }
        if (db_handle->get_uint(5, punish.end_time) != true) {
            return -1;
        }
    }

    return 0;
}

// IDA_EA: 0x818C0E0
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY29arad_sp_convert_pass_pad_infoEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA *)
int arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2B0,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->set_query(
            "seLect mousepass, fail_cnt, cancel_cnt, version_info, validity_time, enable_flag from member_mousepass where m_id=%d",
            static_cast<int>(payload->member_id)) != true) {
        return -1;
    }

    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2BC,
            "%s",
            "arad_sp_convert_pass_pad_info fail");
        return -1;
    }

    if (db_handle->get_n_rows() == 0) {
        cMyTrace trace(
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2C2,
            0);
        trace("Goblin Pad Exist");
        payload->passpad_result = 0;
        return 0;
    }

    if (db_handle->fetch() != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2C9,
            "%s",
            "arad_sp_convert_pass_pad_info fetch fail");
        return -1;
    }

    if (db_handle->get_str(0, payload->passpad_mousepass, 36) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2CF,
            "%s",
            "Mouse Pass Encrypted Parsing error");
        return -1;
    }

    if (db_handle->get_int(1, payload->passpad_fail_count) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2D5,
            "%s",
            "Mouse PassPad Fail Count Parsing error");
        return -1;
    }

    if (db_handle->get_int(2, payload->passpad_cancel_count) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2DB,
            "%s",
            "PassPad Cancel Count Parsing error");
        return -1;
    }

    if (db_handle->get_byte(3, payload->passpad_result) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2E0,
            "%s",
            "VersionInfo Parsing error");
        return -1;
    }

    // SIG_LOGIN_DATA retains the original signed long ABI field, whereas the
    // MySQL API exposes this column as unsigned.  get_uint never changes its
    // output on failure, so the local preserves the old field state until the
    // original success gate is crossed.
    unsigned int validity_time = 0;
    if (db_handle->get_uint(4, validity_time) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2E6,
            "%s",
            "Goblin Validity Time Parsing error");
        return -1;
    }
    payload->passpad_validity_time = static_cast<long>(validity_time);

    char enable_flag = 0;
    if (db_handle->get_byte(5, enable_flag) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA*)",
            0x2ED,
            "%s",
            "Goblin enable flage Parsing error");
        return -1;
    }

    if (enable_flag == 1) {
        payload->passpad_converted = true;
    }
    return 0;
}

// IDA_EA: 0x818C4F3
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY27arad_sp_check_goblin_rewardEP14SIG_LOGIN_DATAPc
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_check_goblin_reward(SIG_LOGIN_DATA *,char *)
int arad_sp_check_goblin_reward(SIG_LOGIN_DATA* payload, char* timestamp) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_check_goblin_reward(SIG_LOGIN_DATA*, char*)",
            0x30D,
            "%s",
            "Database null!!");
        return -1;
    }

    // The old routine deliberately ignores the set_query result and calls
    // exec() unconditionally.
    db_handle->set_query(
        "seLect m_id from member_mousepass_history where modify_type = %d and m_id=%d and occ_time >= '%s'",
        16,
        static_cast<int>(payload->member_id),
        timestamp);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_check_goblin_reward(SIG_LOGIN_DATA*, char*)",
            0x31F,
            "%s",
            "arad_sp_check_goblin_reward fail");
        return -1;
    }

    // The result is intentionally discarded before the two output markers
    // are set; that is the exact old control path.
    (void)db_handle->get_n_rows();
    payload->passpad_enabled = true;
    payload->passpad_result = 2;
    return 0;
}

// IDA_EA: 0x818C609
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY30arad_sp_delete_member_mouspassEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_delete_member_mouspass(SIG_LOGIN_DATA *)
int arad_sp_delete_member_mouspass(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_delete_member_mouspass(SIG_LOGIN_DATA*)",
            0x345,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "deLete from member_mousepass where m_id=%d",
            static_cast<int>(payload->member_id)) != 0) {
        return -1;
    }

    // IDA @0x818C6A6 returns this in EDX:EAX and tests both halves with
    // `or eax, edx`; retain the full unsigned 64-bit zero test.
    const unsigned long long affected_row_count =
        db_handle->getAffectedRowCount();
    return affected_row_count != 0 ? 0 : -1;
}

// IDA_EA: 0x818C6C4
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY26arad_sp_update_fail_recordEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_update_fail_record(SIG_LOGIN_DATA *)
int arad_sp_update_fail_record(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_update_fail_record(SIG_LOGIN_DATA*)",
            0x365,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "upDate member_mousepass set fail_cnt=%d where m_id=%d",
            static_cast<int>(payload->passpad_fail_count),
            static_cast<int>(payload->member_id)) != 0) {
        return -1;
    }

    return 0;
}

// IDA_EA: 0x818C76F
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY22arad_sp_update_historyEP14SIG_LOGIN_DATAi
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_update_history(SIG_LOGIN_DATA *,int)
int arad_sp_update_history(SIG_LOGIN_DATA* payload, int modify_type) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_update_history(SIG_LOGIN_DATA*, int)",
            0x37C,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect mousepass from member_mousepass  where m_id=%d",
            static_cast<int>(payload->member_id)) != 0) {
        return -1;
    }

    char previous_mousepass[36];
    memset(previous_mousepass, 0, sizeof(previous_mousepass));
    if (db_handle->get_str(0, previous_mousepass, 36) != true) {
        return -1;
    }

    if (db_handle->select(
            "inSert into member_mousepass_history(occ_time, m_id, pre_mousepass, modify_type)  values (now(),%d,'%s',%d)",
            static_cast<int>(payload->member_id),
            previous_mousepass,
            modify_type) != 0) {
        return -1;
    }

    // IDA @0x818C895 returns this in EDX:EAX and tests both halves with
    // `or eax, edx`; retain the full unsigned 64-bit zero test.
    const unsigned long long affected_row_count =
        db_handle->getAffectedRowCount();
    return affected_row_count != 0 ? 0 : -1;
}

// IDA_EA: 0x818C8B8
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY24arad_sp_load_event_entryEP14SIG_LOGIN_DATARSt6vectorIN8DB_Login16tagDNFEventEntryESaIS6_EE
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_load_event_entry(SIG_LOGIN_DATA *,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &)
int arad_sp_load_event_entry(
    SIG_LOGIN_DATA* payload,
    std::vector<DB_Login::tagDNFEventEntry>& event_entries) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_load_event_entry(SIG_LOGIN_DATA*, DB_Login::vecDNFEventEntry_t&)",
            0x3a5,
            "%s",
            "Database null!!");
        return -1;
    }

    // This is deliberately the old inverted gate.  A successful set_query()
    // returns -1 immediately; the only fallthrough reads the preceding MySQL
    // row state without exec() or fetch().
    if (db_handle->set_query(
            "seLect event_id,unix_timestamp(occ_date),server_id,charac_no,unix_timestamp(obtain_date) from dnf_event_entry where m_id=%d and server_id in(0,%d)",
            payload->member_id,
            G_CEnvironment()->get_server_group()) == true) {
        return -1;
    }

    for (unsigned int row_index = 0;
         static_cast<unsigned int>(db_handle->get_n_rows()) > row_index;
         ++row_index) {
        // The raw vector copy includes all six words.  The SQL readers leave
        // +0x04 unwritten, so this automatic object must not be value-zeroed.
        DB_Login::tagDNFEventEntry entry;
        if (db_handle->get_int(0, entry.event_id) == true &&
            db_handle->get_int(1, entry.occ_time) == true &&
            db_handle->get_int(2, entry.server_id) == true &&
            db_handle->get_int(3, entry.charac_no) == true &&
            db_handle->get_int(4, entry.obtain_time) == true) {
            event_entries.push_back(entry);
        }
    }
    return 0;
}

// IDA_EA: 0x818CA69
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY30arad_sp_select_member_brodcastEP14SIG_LOGIN_DATARN8DB_Login16tagDNFEventEntryEi
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_brodcast(SIG_LOGIN_DATA *,DB_Login::tagDNFEventEntry &,int)
int arad_sp_select_member_brodcast(
    SIG_LOGIN_DATA* payload,
    DB_Login::tagDNFEventEntry& event_entry,
    int user_idx) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_brodcast(SIG_LOGIN_DATA*, DB_Login::tagDNFEventEntry&, int)",
            0x3d6,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect m_id,charac_no,server_id,unix_timestamp(start_time),charac_name from member_broadcast where event_id=%d and m_id=%d and end_time>now() limit 1",
            payload->member_id,
            event_entry.event_id) != 0) {
        return -1;
    }

    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    unsigned int server_id = 0;
    unsigned int start_time = 0;
    if (db_handle->get_uint(0, account_id) != true ||
        db_handle->get_uint(1, charac_no) != true ||
        db_handle->get_uint(2, server_id) != true ||
        db_handle->get_uint(3, start_time) != true ||
        db_handle->get_str(4, payload->current_charac_name, 30) != true) {
        return -1;
    }

    if (static_cast<int>(start_time) > GlobalData::s_systemTime_.getCurSec()) {
        return -1;
    }

    if (G_GameWorld()->GetChannelType() == 7) {
        DBMgr::stUserDBInfo_t info;
        info.m_id = account_id;
        info.charac_no = charac_no;
        info.server_id = server_id;
        GlobalData::s_db_mgr->addUserDBInfo(user_idx, info);
    }
    payload->current_charac_no = charac_no;
    payload->school_broadcast_server_group = server_id;
    return 0;
}

// IDA_EA: 0x818CC74
// IDA_NAME: _ZN4ARAD8DATABASE11LOGIN_QUERY29arad_sp_select_member_premiumEP14SIG_LOGIN_DATARN8DB_Login16tagDNFEventEntryERSt6vectorIiSaIiEES9_
// DEMANGLED: ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(SIG_LOGIN_DATA *,DB_Login::tagDNFEventEntry &,std::vector<int,std::allocator<int>> &,std::vector<int,std::allocator<int>>)
int arad_sp_select_member_premium(
    SIG_LOGIN_DATA* payload,
    DB_Login::tagDNFEventEntry& event_entry,
    std::vector<int>& /* event_ids */,
    std::vector<int> remove_event_ids) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMemberMousePassDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(SIG_LOGIN_DATA*, DB_Login::tagDNFEventEntry&, IntVector&, ARAD::DATABASE::LOGIN_QUERY::vecPremiumRemoveInfo_t)",
            0x412,
            "%s",
            "Database null!!");
        return -1;
    }

    if (db_handle->select(
            "seLect pre_type,unix_timestamp(service_start),unix_timestamp(service_end) from member_premium where m_id=%d and event_id=%d and service_end>now() and server_id in(0,%d) order by service_end desc limit 1",
            static_cast<int>(payload->member_id),
            event_entry.event_id,
            event_entry.server_id) != 0) {
        return -1;
    }

    if (db_handle->get_n_rows() == 0) {
        // The raw ABI passes this vector by value.  Its direct old caller
        // constructs a temporary copy and destroys it after return, so this
        // mutation is intentionally not a caller-visible removal list.
        remove_event_ids.push_back(event_entry.event_id);
        return 0;
    }

    int premium_count = 0;
    for (unsigned int row_index = 0;; ++row_index) {
        if (static_cast<unsigned int>(db_handle->get_n_rows()) <= row_index ||
            db_handle->fetch() != true) {
            break;
        }

        int premium_type = 0;
        int service_start = 0;
        int service_end = 0;
        if (db_handle->get_int(0, premium_type) != true) {
            cMyTrace trace(
                "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(SIG_LOGIN_DATA*, DB_Login::tagDNFEventEntry&, IntVector&, ARAD::DATABASE::LOGIN_QUERY::vecPremiumRemoveInfo_t)",
                0x432,
                5);
            trace("DB_Login::GetPremiumInfo, get(1) ERROR m_id=%u",
                  payload->member_id);
            return -1;
        }
        if (db_handle->get_int(1, service_start) != true) {
            cMyTrace trace(
                "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(SIG_LOGIN_DATA*, DB_Login::tagDNFEventEntry&, IntVector&, ARAD::DATABASE::LOGIN_QUERY::vecPremiumRemoveInfo_t)",
                0x438,
                5);
            trace("DB_Login::GetPremiumInfo, get(2) ERROR m_id=%u",
                  payload->member_id);
            return -1;
        }
        if (db_handle->get_int(2, service_end) != true) {
            cMyTrace trace(
                "int ARAD::DATABASE::LOGIN_QUERY::arad_sp_select_member_premium(SIG_LOGIN_DATA*, DB_Login::tagDNFEventEntry&, IntVector&, ARAD::DATABASE::LOGIN_QUERY::vecPremiumRemoveInfo_t)",
                0x43e,
                5);
            trace("DB_Login::GetPremiumInfo, get(3) ERROR m_id=%u",
                  payload->member_id);
            return -1;
        }

        if (premium_count > 105) {
            break;
        }

        SIG_LOGIN_DATA::PremiumEntry& premium =
            payload->premium_entries[premium_count];
        premium.event_id = event_entry.event_id;
        premium.premium_type = premium_type;
        premium.service_start = service_start;
        premium.service_end = service_end;
        ++premium_count;

        const uint16_t event_index = payload->premium_event_count;
        payload->premium_event_ids[event_index] = event_entry.event_id;
        payload->premium_event_count = event_index + 1;
    }

    return 0;
}

}  // namespace LOGIN_QUERY
}  // namespace DATABASE
}  // namespace ARAD
