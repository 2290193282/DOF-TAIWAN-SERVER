#include "old_data/db/account/db_login.h"

#include "common/types/compressed_blob.h"
#include "old_data/db/account/charac_view_compat.h"
#include "old_game/player/core/character_load_mercenary.h"
#include "old_game/player/session/online_preliminary_runtime.h"
#include "old_data/db/auth_security/db_mouse_register.h"
#include "old_data/db/core/mysql.h"
#include "old_data/Login/simple_sso.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_data/db/core/db_mgr.h"
#include "common/manager/dnff_lib.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/event_login_item_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/clog.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <new>
#include <vector>
#include <zlib.h>



extern long getDailyInitializeTime(int hour, int minute, int second)
    __asm__("_Z22getDailyInitializeTimeiii");
namespace {

static_assert(sizeof(SIG_LOGIN_DATA) == 0x4478,
              "SIG_LOGIN_DATA must match the DB login stream payload");

enum {
    kSigLoginDataSize = sizeof(SIG_LOGIN_DATA),
    kInterUpdateLoginSignal = 43,
    kMemberInfoPasswordMax = 132,
    kCRijndaelSize = 0x47C,
    kCRijndaelSuccess = 1879048191,
    kMaxPunishRows = 50,
    kCharacViewCountMax = 36,
    kLegacyCharacViewBlobSize =
        sizeof(oldsrv::db::charac_view_compat::LegacyCharacViewEntry) *
        kCharacViewCountMax,
    kNativeCharacViewBlobSize =
        sizeof(SIG_LOGIN_DATA::CharacViewEntry) * kCharacViewCountMax,
};

const unsigned int kCompressedBlobRawSizeMask = 0x3FFFFFFFu;

void LogCharacViewSlot(int slot,
                       const SIG_LOGIN_DATA::CharacViewEntry& entry,
                       const char* skip_reason) {
    const unsigned int raw_grow = entry.identity.grow_type;
    LogManager::logFormat(
        1,
        "charac_view_diag.cpp",
        "DB_Login::_getGameDBCharacView",
        0,
        "[CHARAC_VIEW][SLOT] slot=%d job=%u raw_grow=%u first_grow=%u "
        "second_grow=%u skip_reason=%s",
        slot,
        static_cast<unsigned int>(entry.identity.job),
        raw_grow,
        raw_grow & 0x0Fu,
        (raw_grow >> 4u) & 0x07u,
        skip_reason ? skip_reason : "unknown");
}

bool ReadCharacViewCompressedBlob(MySQL* db_handle,
                                  int column,
                                  std::vector<char>* encoded,
                                  unsigned int* raw_size) {
    if (!db_handle || !encoded || !raw_size) {
        return false;
    }

    const int encoded_size = db_handle->get_binary_length(column);
    if (encoded_size < 0) {
        return false;
    }
    if (encoded_size == 0) {
        encoded->clear();
        *raw_size = 0;
        return true;
    }
    if (encoded_size < 4 ||
        static_cast<uLong>(encoded_size) >
            compressBound(kNativeCharacViewBlobSize) + 4u) {
        return false;
    }

    encoded->resize(static_cast<size_t>(encoded_size));
    if (db_handle->get_binary(column, &(*encoded)[0], encoded_size) != true) {
        encoded->clear();
        return false;
    }
    unsigned int header = 0;
    memcpy(&header, &(*encoded)[0], sizeof(header));
    *raw_size = header & kCompressedBlobRawSizeMask;
    if (*raw_size != kLegacyCharacViewBlobSize &&
        *raw_size != kNativeCharacViewBlobSize) {
        encoded->clear();
        return false;
    }
    return true;
}

bool DecodeCharacViewCompressedBlob(const std::vector<char>& encoded,
                                    unsigned int expected_raw_size,
                                    char* destination) {
    if (encoded.size() < 4 || !destination) {
        return false;
    }
    uLongf decoded_size = static_cast<uLongf>(expected_raw_size);
    return ::uncompress(
               reinterpret_cast<Bytef*>(destination),
               &decoded_size,
               reinterpret_cast<const Bytef*>(&encoded[0] + 4),
               static_cast<uLong>(encoded.size() - 4)) == Z_OK &&
           decoded_size == static_cast<uLongf>(expected_raw_size);
}

bool RewriteNativeCharacViewBlob(MySQL* db_handle,
                                 unsigned int member_id,
                                 char* native_blob,
                                 const std::vector<char>& original_blob) {
    if (!db_handle || !native_blob || original_blob.empty()) {
        return false;
    }

    char compressed[6405];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_size = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_size,
                     native_blob,
                     kNativeCharacViewBlobSize) != 1) {
        return false;
    }
    char* const native_escaped =
        db_handle->blob_to_str(0, compressed, compressed_size);
    char* const original_escaped = db_handle->blob_to_str(
        1,
        const_cast<char*>(&original_blob[0]),
        static_cast<int>(original_blob.size()));
    if (!native_escaped || !original_escaped) {
        return false;
    }

    // Compare-and-swap prevents a concurrent character-view save from being
    // overwritten by this one-time login migration.
    if (!db_handle->set_query(
            "upDate charac_view set info='%s' where m_id=%s and info='%s'",
            native_escaped,
            NumberToString(member_id, 0),
            original_escaped)) {
        return false;
    }
    return db_handle->exec(true) == true &&
           db_handle->getAffectedRowCount() == 1u;
}

}  // namespace

// IDA_EA: 0x818D8D0
// IDA_NAME: _ZN8DB_Login16tagDNFEventEntryD2Ev
// DEMANGLED: DB_Login::tagDNFEventEntry::~tagDNFEventEntry()
DB_Login::tagDNFEventEntry::~tagDNFEventEntry() {}

// IDA_EA: 0x840F48E
// IDA_NAME: _ZN8DB_LoginC2Ev
// DEMANGLED: DB_Login::DB_Login(void)
DB_Login::DB_Login() {}

// IDA_EA: 0x840F4AA
// IDA_NAME: _ZN8DB_LoginD2Ev
// DEMANGLED: DB_Login::~DB_Login()
// IDA_EA: 0x840F4DA
// IDA_NAME: _ZN8DB_LoginD0Ev
// DEMANGLED: DB_Login::~DB_Login() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_Login::~DB_Login() {}

// IDA_EA: 0x840F4F8
// IDA_NAME: _ZN8DB_Login10_getGameDBE18ENUM_DB_HANDLE_IDX
// DEMANGLED: DB_Login::_getGameDB(ENUM_DB_HANDLE_IDX)
MySQL* DB_Login::_getGameDB(ENUM_DB_HANDLE_IDX handle_idx) {
    return GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(handle_idx),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
}

// IDA_EA: 0x840F51C
// IDA_NAME: _ZN8DB_Login8dispatchEiiP6Stream
// DEMANGLED: DB_Login::dispatch(int,int,Stream *)
bool DB_Login::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    if (!stream) {
        return false;
    }
    SIG_LOGIN_DATA payload_data;
    SIG_LOGIN_DATA* payload = &payload_data;
    if (stream->get_binary(payload, sizeof(payload_data)) != 1) {
        return false;
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        G_GameWorld()->GetChannelType() != 14) {
        DBMgr::stUserDBInfo_t info;
        info.m_id = 0;
        info.charac_no = 0;
        info.server_id =
            static_cast<uint32_t>(payload->server_group);
        if (GlobalData::s_db_mgr) {
            GlobalData::s_db_mgr->addUserDBInfo(user_idx, info);
            GlobalData::s_db_mgr->setUserIdx(user_idx);
        }
    }

    if (GetAccountInfo(payload) != true) {
        payload->fail_code = 1;
        return SendResult(signal, user_idx, payload);
    }

    SaveLoginTime(payload->member_id,
                  payload->token_timestamp,
                  payload->login_ip);
    payload->punish_second_code = 0;

    if (GetPunishUserInfo(payload) != true) {
        payload->fail_code = 15;
        return SendResult(signal, user_idx, payload);
    }

    payload->punish_user_marker = 0;
    payload->punish_second_trade_count = 0;
    payload->punish_second_trade_gold = 0;
    const unsigned int punish_count = payload->punish_count;
    for (unsigned int index = 0; index < punish_count; ++index) {
        const SIG_LOGIN_DATA::PunishRow& row = payload->punish_rows[index];
        const int punish_type = row.punish_type;
        const int punish_value = row.punish_value;
        const int apply_flag = row.apply_flag;
        const unsigned int end_time = row.end_time;
        CSystemTime* system_time = GlobalData::system_time();
        const int cur_sec = system_time ? system_time->getCurSec() : 0;

        if (punish_type == 1) {
            if (static_cast<int>(end_time) >= cur_sec || !end_time) {
                if (end_time) {
                    int value = punish_value;
                    if (value > 25) {
                        value = 25;
                    }
                    payload->punish_duration = value;
                } else if (apply_flag == 1) {
                    payload->fail_code = 10;
                } else if (apply_flag == 2) {
                    payload->fail_code = 11;
                }
                return SendResult(signal, user_idx, payload);
            }
        }
        if (punish_type == 2 || punish_type == 1002) {
            if (static_cast<int>(end_time) >= cur_sec || !end_time) {
                if (end_time) {
                    int value = punish_value;
                    if (value > 25) {
                        value = 25;
                    }
                    payload->fail_code = value + 12;
                } else if (apply_flag == 1) {
                    payload->fail_code = 10;
                } else if (apply_flag == 2) {
                    payload->fail_code = 11;
                }
                return SendResult(signal, user_idx, payload);
            }
        }
        if (punish_type == 3 || punish_type == 1003) {
            if (end_time) {
                int value = punish_value;
                if (value > 25) {
                    value = 25;
                }
                payload->fail_code = value + 12;
            } else if (apply_flag == 1) {
                payload->fail_code = 10;
            } else if (apply_flag == 2) {
                payload->fail_code = 11;
            }
            return SendResult(signal, user_idx, payload);
        }
        if (punish_type == 4) {
            if (apply_flag == 1) {
                payload->punish_second_code = 13;
            } else if (apply_flag == 2) {
                payload->punish_second_code = 14;
            }
        }
        if (punish_type == 10) {
            payload->punish_user_marker = 1;
        }
    }

    _getRewardInfo(payload);
    int passpad_result = 0;
    if (payload->passpad_enabled && payload->passpad_converted != 1) {
        passpad_result = _convertPassPadInfo(payload);
    }
    if (passpad_result) {
        if (passpad_result == 72) {
            payload->fail_code = 5;
        } else if (passpad_result == 166) {
            payload->fail_code = 9;
        } else {
            payload->fail_code = 6;
        }
        return SendResult(signal, user_idx, payload);
    }

    payload->fail_code = 0;
    vecDNFEventEntry_t entries;
    _LoadDNFEventEntry(payload->member_id, entries);
    GetPremiumInfo(user_idx, payload, entries);

    bool blocked_channel = false;
    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type == 7 &&
        (!payload->current_charac_no || !payload->school_broadcast_server_group)) {
        blocked_channel = true;
    } else if (channel_type == 14) {
        CDataManager* data_manager = G_CDataManager();
        const int pvp_mode = data_manager ? data_manager->GetPvPMode() : 0;
        const bool allow_pvp =
            payload->online_preliminary_charac_no &&
            payload->online_preliminary_server_group &&
            ((payload->pvp_mode_mask & pvp_mode) != 1);
        if (!allow_pvp) {
            blocked_channel = true;
        }
    }

    if (blocked_channel) {
        payload->fail_code = 8;
        return SendResult(signal, user_idx, payload);
    }

    if (GetGameDBInfo(payload) == true) {
        payload->fail_code = 15;
    } else {
        UpdateLoginStat(user_idx, payload);
    }
    if (payload->security_card_loaded && payload->security_card_valid != 1) {
        _getSecurityCardInfo(payload);
    }
    getCharacLinkSystemData(payload);
    payload->break_away_member = getBreakAwayMemberData(payload) == true;
    payload->school_point_joined =
        _isJoinedSchoolPointEvent(payload->member_id);
    GetDailyCleanPadPoint(payload);
    IsThisIPLoginPermissionGranted(payload);
    GetReturnUserInfo(payload);
    GetLoginCommonData(payload);
    GetCheckPickUpRandomOptionItem(payload);
    GetOntimeUserRecvIdx(payload);
    GetUserFatigueCount(payload);
    GetUserInGameEventHistory(payload);
    CheckLoginIP(payload);
    GetMercenarySystemInfo(payload);
    GetBingoEvent(payload);
    GetGrowthWeaponEvent(payload);
    GetGrowthCreatureEvent(payload);
    GetAccountCharacterLevelUpOnceGift(payload);
    return SendResult(signal, user_idx, payload);
}

// IDA_EA: 0x840FFCA
// IDA_NAME: _ZN8DB_Login23getCharacLinkSystemDataEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::getCharacLinkSystemData(SIG_LOGIN_DATA *)
bool DB_Login::getCharacLinkSystemData(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id = NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "seLect message_flag from charac_link_message where m_id=%s",
        account_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "void DB_Login::getCharacLinkSystemData(SIG_LOGIN_DATA*)",
                              12116,
                              "LINK_LOG : charac_link_message DB ERROR!!");
        return false;
    }
    if (!db_handle->get_n_rows()) {
        account_id = NumberToString(payload->member_id, 0);
        db_handle->set_query(
            "inSert into charac_link_message(m_id,message_flag) values(%s,0)",
            account_id);
        if (db_handle->exec(true) != true) {
            LogManager::logFormat(1,
                                  "DBThread.cpp",
                                  "void DB_Login::getCharacLinkSystemData(SIG_LOGIN_DATA*)",
                                  12125,
                                  "LINK_LOG : charac_link_message DB INSERT FAIL!!");
            return false;
        }
        return false;
    }
    if (db_handle->get_n_rows() != 1 || db_handle->fetch() != true) {
        cMyTrace trace("void DB_Login::getCharacLinkSystemData(SIG_LOGIN_DATA*)",
                       12133,
                       5);
        trace("LINK_LOG : charac_link_message DB ERROR!! M_ID(%s)",
              NumberToString(payload->member_id, 0));
        return false;
    }
    return db_handle->get_byte(0, payload->charac_link_message) != true;
}

// IDA_EA: 0x84101AE
// IDA_NAME: _ZN8DB_Login22getBreakAwayMemberDataEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA *)
bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id =
        NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "seLect accrue_cera, play_info, last_reward_time, first_reward_time, luck_point, last_update_time, add_info, second_reward_time, quest_time from churn_member_info where m_id=%s",
        account_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                              12411,
                              "BREAK_AWAY : churn_member_info DB ERROR!!");
        return false;
    }
    if (!db_handle->get_n_rows()) {
        char play_info[31];
        memset(play_info, '0', sizeof(play_info));
        play_info[30] = 0;
        play_info[28] = '1';
        account_id =
            NumberToString(payload->member_id, 0);
        db_handle->set_query(
            "inSert into churn_member_info(m_id,accrue_cera,play_info,last_reward_time,first_reward_time,server_id,charac_no,item_id,add_info,luck_point,last_update_time,second_reward_time) values(%s,0,'%s',0,0,0,0,0,0,0,0,0)",
            account_id,
            play_info);
        if (db_handle->exec(true) != true) {
            LogManager::logFormat(1,
                                  "DBThread.cpp",
                                  "bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                                  12428,
                                  "BREAK_AWAY : churn_member_info DB INSERT FAIL!!");
            return false;
        }
        memset(&payload->break_away_accrue_cera, 0, 0xA4);
        memcpy(payload->break_away_play_info, play_info, sizeof(play_info));
        return true;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                       12442,
                       5);
        trace("BREAK_AWAY : churn_member_info NO MEMBER USER!! M_ID(%s)",
              NumberToString(payload->member_id, 0));
        return false;
    }
    if (db_handle->get_int(0, payload->break_away_accrue_cera) != true) return false;
    if (db_handle->get_str(1, payload->break_away_play_info, 31) != true) return false;
    if (!payload->break_away_play_info[0]) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                              12459,
                              "BREAK_AWAY : ABNOMAL PLAY_INFO DATA!! M_ID(%s)",
                              NumberToString(payload->member_id, 0));
        return false;
    }
    if (db_handle->get_uint(2, payload->break_away_last_reward_time) != true) return false;
    if (db_handle->get_uint(3, payload->break_away_first_reward_time) != true) return false;
    if (db_handle->get_int(4, payload->break_away_luck_point) != true) return false;
    if (db_handle->get_uint(5, payload->break_away_last_update_time) != true) return false;
    if (db_handle->get_int(6, payload->break_away_add_info) != true) return false;
    if (db_handle->get_uint(7, payload->break_away_second_reward_time) != true) return false;
    if (db_handle->get_uint(8, payload->break_away_quest_time) != true) return false;

    if (payload->break_away_last_reward_time) {
        const int previous_schedule_time =
            getPrevScheduleTime(0, payload->break_away_last_reward_time);
        CSystemTime* system_time = GlobalData::system_time();
        const int cur_sec = system_time ? system_time->getCurSec() : 0;
        int pass_days = (cur_sec - previous_schedule_time) / 86400;
        if (pass_days < 0) {
            pass_days = 0;
        }
        if (pass_days && payload->break_away_luck_point) {
            payload->break_away_luck_point = 0;
            account_id = NumberToString(payload->member_id, 0);
            db_handle->set_query(
                "upDate churn_member_info set luck_point=%d where m_id=%s",
                payload->break_away_luck_point,
                account_id);
            if (db_handle->exec(true) != true ||
                !db_handle->getAffectedRowCount()) {
                LogManager::logFormat(1,
                                      "DBThread.cpp",
                                      "bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                                      12497,
                                      "BREAK_AWAY : LUCK POINT UPDATE ERROR!!");
                return false;
            }
        }
    }

    CSystemTime* system_time = GlobalData::system_time();
    const int cur_sec = system_time ? system_time->getCurSec() : 0;
    const bool quest_time_expired =
        payload->break_away_quest_time &&
        static_cast<int>(payload->break_away_quest_time) < cur_sec &&
        (payload->break_away_last_reward_time ==
             payload->break_away_first_reward_time ||
         payload->break_away_last_reward_time ==
             payload->break_away_second_reward_time);
    if (quest_time_expired) {
        payload->break_away_add_info = -1;
        payload->break_away_quest_time = 0;
        db_handle->set_query(
            "upDate churn_member_info set add_info=%d, quest_time=0 where m_id=%u",
            payload->break_away_add_info,
            payload->member_id);
        if (db_handle->exec(true) != true ||
            !db_handle->getAffectedRowCount()) {
            LogManager::logFormat(1,
                                  "DBThread.cpp",
                                  "bool DB_Login::getBreakAwayMemberData(SIG_LOGIN_DATA*)",
                                  12518,
                                  "BREAK_AWAY : ADD INFO UPDATE ERROR!!");
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x841090C
// IDA_NAME: _ZN8DB_Login18GetLoginCommonDataEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetLoginCommonData(SIG_LOGIN_DATA *)
bool DB_Login::GetLoginCommonData(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id =
        NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "seLect  member_bonus_fatigue , update_time , login_ip , inform_notice  from login_common where m_id=%s",
        account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        payload->return_user_bonus_fatigue = 0;
        memset(payload->previous_login_ip, 0, sizeof(payload->previous_login_ip));
        memset(payload->login_event_state, 0, 16);
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    int bonus_fatigue = 0;
    int update_time = 0;
    if (db_handle->get_int(0, bonus_fatigue) != true) return false;
    if (db_handle->get_int(1, update_time) != true) return false;
    payload->return_user_update_time = static_cast<unsigned int>(update_time);
    payload->return_user_bonus_fatigue = static_cast<unsigned short>(bonus_fatigue);
    if (db_handle->get_str(2, payload->previous_login_ip, 16) != true) return false;
    return true;
}

// IDA_EA: 0x8410AD8
// IDA_NAME: _ZN8DB_Login17GetReturnUserInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetReturnUserInfo(SIG_LOGIN_DATA *)
bool DB_Login::GetReturnUserInfo(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id =
        NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "seLect unix_timestamp(expire_time), first_login from return_user where m_id=%s and expire_time > now()",
        account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        payload->return_user_loaded = 0;
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    int first_login = 0;
    if (db_handle->get_int(0, payload->return_user_value) != true) return false;
    if (db_handle->get_int(1, first_login) != true) return false;
    payload->return_user_first_login = first_login == 0;
    payload->return_user_loaded = 1;
    return true;
}

// IDA_EA: 0x8410C12
// IDA_NAME: _ZN8DB_Login21GetDailyCleanPadPointEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetDailyCleanPadPoint(SIG_LOGIN_DATA *)
bool DB_Login::GetDailyCleanPadPoint(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id =
        NumberToString(payload->member_id, 0);
    db_handle->set_query("seLect cleanpad_point from member_login where m_id=%s",
                         account_id);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    return db_handle->get_uint(0, payload->daily_clean_pad_count) == true;
}

// IDA_EA: 0x8410CF6
// IDA_NAME: _ZN8DB_Login17GetPunishUserInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetPunishUserInfo(SIG_LOGIN_DATA *)
bool DB_Login::GetPunishUserInfo(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }

    const char* account_id =
        NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "seLect punish_type, unix_timestamp(occ_time), punish_value, apply_flag, unix_timestamp(start_time), unix_timestamp(end_time) from member_punish_info where m_id=%s and apply_flag>=1 LIMIT %d",
        account_id,
        kMaxPunishRows);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return true;
    }
    int row_count = db_handle->get_n_rows();
    if (row_count > kMaxPunishRows) {
        row_count = kMaxPunishRows;
    }
    payload->punish_count =
        static_cast<unsigned int>(row_count);
    for (int row = 0; row < row_count; ++row) {
        SIG_LOGIN_DATA::PunishRow& entry = payload->punish_rows[row];
        if (db_handle->fetch() != true) return false;
        if (db_handle->get_int(0, entry.punish_type) != true) return false;
        if (db_handle->get_uint(1, entry.occurrence_time) != true) return false;
        if (db_handle->get_int(2, entry.punish_value) != true) return false;
        if (db_handle->get_int(3, entry.apply_flag) != true) return false;
        if (db_handle->get_uint(4, entry.start_time) != true) return false;
        if (db_handle->get_uint(5, entry.end_time) != true) return false;
    }
    return true;
}

// IDA_EA: 0x8410F74
// IDA_NAME: _ZN8DB_Login25_isJoinedSchoolPointEventEj
// DEMANGLED: DB_Login::_isJoinedSchoolPointEvent(unsigned int)
bool DB_Login::_isJoinedSchoolPointEvent(unsigned int account_id) {
    CSystemTime* system_time = GlobalData::system_time();
    if (system_time && system_time->getCurSec() > 1211410800) {
        return false;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query("seLect school_id from event_school_point where m_id=%s",
                         NumberToString(account_id, 0));
    return db_handle->exec(true) == true && db_handle->get_n_rows() != 0;
}

// IDA_EA: 0x841102E
// IDA_NAME: _ZN8DB_Login24_isAutoPunishedFirstUserEj
// DEMANGLED: DB_Login::_isAutoPunishedFirstUser(unsigned int)
bool DB_Login::_isAutoPunishedFirstUser(unsigned int account_id) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect punish_flag from auto_punish_first_user where m_id=%s and punish_flag=1",
        NumberToString(account_id, 0));
    return db_handle->exec(true) == true && db_handle->get_n_rows() != 0;
}

// IDA_EA: 0x84110C6
// IDA_NAME: _ZN8DB_Login30_getAutoPunishedSecondUserDataEjRjRy
// DEMANGLED: DB_Login::_getAutoPunishedSecondUserData(unsigned int,unsigned int &,unsigned long long &)
bool DB_Login::_getAutoPunishedSecondUserData(unsigned int account_id,
                                              unsigned int& trade_count,
                                              unsigned long long& trade_gold) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect trade_cnt,trade_gold,punish_flag,unix_timestamp(occ_time) from auto_punish_second_user where m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    char punish_flag = 0;
    int occ_time = 0;
    db_handle->get_uint(0, trade_count);
    db_handle->get_ulonglong(1, trade_gold);
    db_handle->get_byte(2, punish_flag);
    db_handle->get_int(3, occ_time);
    return punish_flag != 0;
}

// IDA_EA: 0x84111E2
// IDA_NAME: _ZN8DB_Login20_getSecurityCardInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::_getSecurityCardInfo(SIG_LOGIN_DATA *)
int DB_Login::_getSecurityCardInfo(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return 0;
    }
    if (!payload->security_card_service) {
        payload->security_card_loaded = 0;
        return 0;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        payload->security_card_loaded = 0;
        return 0;
    }
    db_handle->set_query(
        "seLect card,fail_cnt,validity_time>unix_timestamp(now()),cert_flag, cert_cnt, q_pos from member_security_card where m_id=%s and apply_flag=1",
        NumberToString(payload->member_id, 0));
    db_handle->exec(true);
    if (!db_handle->get_n_rows()) {
        payload->security_card_loaded = 0;
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 0;
    }

    unsigned int valid_time = 0;
    db_handle->get_str(0, payload->security_card, 39);
    db_handle->get_int(1, payload->security_card_fail_count);
    db_handle->get_uint(2, valid_time);
    db_handle->get_uint(3, payload->security_card_cancel_count);
    db_handle->get_byte(4, payload->security_card_cert_flag);
    db_handle->get_short(5, payload->security_card_q_pos);
    db_handle->get_str(5, payload->security_card_q_pos_text, 5);
    if (valid_time == 1) {
        payload->security_card_valid = 1;
    }
    payload->security_card_loaded = 1;

    int reset_cancel_count = 0;
    CSystemTime* system_time = GlobalData::system_time();
    CEnvironment* environment = G_CEnvironment();
    if (payload->security_card_cancel_count && system_time && environment) {
        reset_cancel_count =
            CheckDailyScheduleTime(environment->get_fatigue_time(),
                                   payload->last_play_time,
                                   system_time->getCurSec()) ? 1 : 0;
    }
    if (reset_cancel_count) {
        payload->security_card_cancel_count = 0;
        db_handle->set_query("upDate member_security_card set cancel_cnt=0 where m_id=%s",
                             NumberToString(payload->member_id, 0));
        return db_handle->exec(true);
    }
    return 0;
}

// IDA_EA: 0x841144A
// IDA_NAME: _ZN8DB_Login15_getPassPadInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::_getPassPadInfo(SIG_LOGIN_DATA *)
int DB_Login::_getPassPadInfo(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return 1;
    }
    payload->passpad_enabled = 0;
    payload->passpad_cancel_count = 0;
    payload->passpad_fail_count = 0;
    memset(payload->passpad_mousepass, 0, 0x24);
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return 1;
    }
    db_handle->set_query(
        "seLect mousepass,fail_cnt,cancel_cnt from member_mousepass where m_id=%s and enable_flag = '1'",
        NumberToString(payload->member_id, 0));
    if (db_handle->exec(true) != true) {
        return 1;
    }
    if (!db_handle->get_n_rows()) {
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 1;
    }
    if (db_handle->get_str(0, payload->passpad_mousepass, 36) != true) {
        return 1;
    }
    if (db_handle->get_int(1, payload->passpad_fail_count) != true) {
        return 1;
    }
    if (db_handle->get_int(2, payload->passpad_cancel_count) != true) {
        return 1;
    }
    payload->passpad_enabled = 1;
    if (payload->passpad_fail_count > 2) {
        return 73;
    }
    return 0;
}

// IDA_EA: 0x84115F8
// IDA_NAME: _ZN8DB_Login14_getRewardInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::_getRewardInfo(SIG_LOGIN_DATA *)
int DB_Login::_getRewardInfo(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return 0;
    }
    payload->reward_security_flags = 0;
    payload->reward_daily_reset = 0;
    payload->reward_state = 0;
    payload->passpad_enabled = 0;
    payload->passpad_converted = 0;
    payload->security_card_loaded = 0;
    payload->security_card_valid = 0;

    MySQL* db_handle = _GetDNFAccountDB();
    if (!db_handle) {
        return 0;
    }
    db_handle->set_query(
        "seLect unix_timestamp(last_check_time), unix_timestamp(m_opt_reg), unix_timestamp(pc_opt_reg), unix_timestamp(security_card_reg), unix_timestamp(goblin_pass_mod), unix_timestamp(member_pc_reg), unix_timestamp(gatekeeper_otp_reg), goblin_validity_time>unix_timestamp(now()), security_card_validity_time>unix_timestamp(now()), validity_ip from member_security_grade where m_id = %s ",
        NumberToString(payload->member_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 0;
    }

    unsigned int last_check_time = 0;
    unsigned int mouse_pass = 0;
    unsigned int pc_otp = 0;
    unsigned int security_card = 0;
    unsigned int passpad_mod = 0;
    unsigned int member_pc = 0;
    unsigned int gatekeeper = 0;
    unsigned int passpad_valid = 0;
    unsigned int card_valid = 0;
    char valid_ip[16];
    memset(valid_ip, 0, sizeof(valid_ip));
    db_handle->get_uint(0, last_check_time);
    db_handle->get_uint(1, mouse_pass);
    db_handle->get_uint(2, pc_otp);
    db_handle->get_uint(3, security_card);
    db_handle->get_uint(4, passpad_mod);
    db_handle->get_uint(5, member_pc);
    db_handle->get_uint(6, gatekeeper);
    db_handle->get_uint(7, passpad_valid);
    db_handle->get_uint(8, card_valid);
    db_handle->get_str(9, valid_ip, sizeof(valid_ip));

    const bool valid_ip_match =
        strncmp(valid_ip, payload->login_ip, 16) == 0;
    if (passpad_mod) {
        payload->passpad_enabled = 1;
    }
    if (passpad_valid && valid_ip_match) {
        payload->passpad_converted = 1;
        payload->passpad_result = 2;
    }
    if (security_card) {
        payload->security_card_loaded = 1;
    }
    if (card_valid && valid_ip_match) {
        payload->security_card_valid = 1;
    }
    unsigned char flags = 0;
    if (mouse_pass) flags |= 1;
    if (pc_otp) flags |= 2;
    if (member_pc) flags |= 4;
    if (security_card) flags |= 8;
    if (passpad_mod) flags |= 0x10;
    if (gatekeeper) flags |= 0x20;
    payload->reward_security_flags = flags;
    if (flags && last_check_time < static_cast<unsigned int>(getDailyInitializeTime(6, 0, 0))) {
        payload->reward_daily_reset = 1;
    }
    return 0;
}

// IDA_EA: 0x8411AF0
// IDA_NAME: _ZN8DB_Login19_convertPassPadInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::_convertPassPadInfo(SIG_LOGIN_DATA *)
int DB_Login::_convertPassPadInfo(SIG_LOGIN_DATA* payload) {
    return DB_MouseRegister::convertPassPadInfo(
        payload->member_id,
        payload->encrypted_password,
        payload->passpad_fail_count,
        payload->passpad_result,
        payload->passpad_cancel_count,
        payload->passpad_enabled,
        payload->passpad_mousepass,
        payload->passpad_converted,
        payload->sso_login_ip,
        payload->passpad_validity_time,
        payload->advice_change_password);
}

// IDA_EA: 0x8411BAE
// IDA_NAME: _ZN8DB_Login16_GetDNFAccountDBEv
// DEMANGLED: DB_Login::_GetDNFAccountDB(void)
MySQL* DB_Login::_GetDNFAccountDB() {
    return GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
}

// IDA_EA: 0x8411BDA
// IDA_NAME: _ZN8DB_Login18_LoadDNFEventEntryEjRSt6vectorINS_16tagDNFEventEntryESaIS1_EE
// DEMANGLED: DB_Login::_LoadDNFEventEntry(unsigned int,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &)
bool DB_Login::_LoadDNFEventEntry(unsigned int account_id,
                                  vecDNFEventEntry_t& entries) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect event_id,unix_timestamp(occ_date),server_id,charac_no,unix_timestamp(obtain_date) from dnf_event_entry where m_id=%s and server_id in(0,%d)",
        NumberToString(account_id, 0),
        // IDA @0x8411BDA reads G_CEnvironment()+0x378, the old
        // CEnvironment::get_server_group() field, not server_type.
        G_CEnvironment() ? G_CEnvironment()->get_server_group() : 0);
    if (db_handle->exec(true) != true) {
        return false;
    }
    const int rows = db_handle->get_n_rows();
    for (int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }
        tagDNFEventEntry entry;
        if (db_handle->get_int(0, entry.event_id) != true) return false;
        if (db_handle->get_int(1, entry.occ_time) != true) return false;
        if (db_handle->get_int(2, entry.server_id) != true) return false;
        if (db_handle->get_int(3, entry.charac_no) != true) return false;
        if (db_handle->get_int(4, entry.obtain_time) != true) return false;
        entries.push_back(entry);
    }
    return true;
}

// IDA_EA: 0x8411D7E
// IDA_NAME: _ZN8DB_Login14_LoadEventItemEP14SIG_LOGIN_DATAiRSt6vectorINS_16tagDNFEventEntryESaIS3_EE
// DEMANGLED: DB_Login::_LoadEventItem(SIG_LOGIN_DATA *,int,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &)
bool DB_Login::_LoadEventItem(SIG_LOGIN_DATA* payload,
                              int event_id,
                              vecDNFEventEntry_t& entries) {
    int event_flags[10];
    memset(event_flags, 0, sizeof(event_flags));
    for (vecDNFEventEntry_t::const_iterator it = entries.begin();
         it != entries.end();
         ++it) {
        if (it->event_id <= 9) {
            event_flags[it->event_id] = 1;
        }
    }

    const unsigned int account_id = payload->member_id;
    for (int index = 0; index <= 9; ++index) {
        if (!event_flags[index]) {
            continue;
        }
        switch (index) {
        case 2:
            WongWork::EventATI::ArrangeATIEventItem(account_id, event_id);
            break;
        case 3:
            WongWork::EventSigong::ArrangeSigongEventItem(account_id, event_id);
            break;
        case 5:
            WongWork::EventMuz::ArrangeMuzEventItem(account_id, event_id);
            break;
        case 6:
            WongWork::EventAlttle::ArrangeAlttleEventItem(account_id, event_id);
            break;
        case 7:
            WongWork::EventFCCombo::ArrangeFCComboEventItem(account_id, event_id);
            break;
        default:
            break;
        }
    }
    return true;
}

// IDA_EA: 0x8411EE8
// IDA_NAME: _ZN8DB_Login14GetPremiumInfoEiP14SIG_LOGIN_DATARSt6vectorINS_16tagDNFEventEntryESaIS3_EE
// DEMANGLED: DB_Login::GetPremiumInfo(int,SIG_LOGIN_DATA *,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &)
void DB_Login::GetPremiumInfo(int user_idx,
                              SIG_LOGIN_DATA* payload,
                              vecDNFEventEntry_t& entries) {
    MySQL* game_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    MySQL* account_db = _GetDNFAccountDB();
    std::vector<int> remove_ids;
    int premium_count = 0;

    payload->premium_count = 0;
    payload->premium_event_count = 0;
    payload->daily_clean_pad_point = 0;

    for (vecDNFEventEntry_t::iterator it = entries.begin();
         it != entries.end(); ++it) {
        const int event_id = it->event_id;
        if (event_id > 41000 && event_id <= 41003) {
            if (!account_db) {
                break;
            }
            account_db->set_query(
                "seLect m_id,charac_no,server_id,unix_timestamp(start_time),charac_name from member_broadcast where event_id=%d and m_id=%s and end_time>now() limit 1",
                event_id,
                NumberToString(payload->member_id, 0));
            account_db->exec(true);
            if (!account_db->get_n_rows()) {
                remove_ids.push_back(event_id);
                continue;
            }
            if (account_db->fetch() != true) {
                break;
            }

            unsigned int account_id = 0;
            unsigned int charac_no = 0;
            unsigned int server_id = 0;
            unsigned int start_time = 0;
            if (account_db->get_uint(0, account_id) == true &&
                account_db->get_uint(1, charac_no) == true &&
                account_db->get_uint(2, server_id) == true &&
                account_db->get_uint(3, start_time) == true) {
                bool channel_match = true;
                const int channel_type = G_GameWorld()->GetChannelType();
                if (channel_type == 7) {
                    channel_match =
                        event_id == 41001 &&
                        account_db->get_str(
                            4,
                            payload->current_charac_name,
                            30) == true;
                } else if (channel_type == 14) {
                    const int play_count =
                        online_preliminary::COnlinePreliminary::GetPlayCountPerOneTeam();
                    channel_match =
                        (event_id == 41002 && play_count == 1) ||
                        (event_id == 41003 && play_count != 1);
                }

                CSystemTime* system_time = GlobalData::system_time();
                const int cur_sec = system_time ? system_time->getCurSec() : 0;
                if (channel_match && static_cast<int>(start_time) <= cur_sec) {
                    if (channel_type == 7 || channel_type == 14) {
                        DBMgr::stUserDBInfo_t info;
                        info.m_id = account_id;
                        info.charac_no = charac_no;
                        info.server_id = server_id;
                        if (GlobalData::s_db_mgr) {
                            GlobalData::s_db_mgr->addUserDBInfo(user_idx, info);
                        }
                    }
                    if (event_id == 41002) {
                        payload->online_preliminary_charac_no =
                            charac_no;
                        payload->online_preliminary_server_group =
                            server_id;
                        payload->pvp_mode_mask |= 1u;
                    } else if (event_id == 41003) {
                        payload->online_preliminary_charac_no =
                            charac_no;
                        payload->online_preliminary_server_group =
                            server_id;
                        payload->pvp_mode_mask |= 2u;
                    } else {
                        payload->current_charac_no =
                            charac_no;
                        payload->school_broadcast_server_group =
                            server_id;
                    }
                }
            }
        } else if (event_id > 49999 && event_id <= 89999) {
            const bool direct_premium =
                (event_id > 69999 && event_id <= 89999) ||
                event_id == 59000 ||
                WongWork::CUserPremium::IsRestrictedPremium(event_id);
            if (direct_premium) {
                const unsigned short index = payload->premium_event_count;
                payload->premium_event_ids[index] = event_id;
                payload->premium_event_count = index + 1;
                continue;
            }

            if (!game_db) {
                continue;
            }
            game_db->set_query(
                "seLect pre_type,unix_timestamp(service_start),unix_timestamp(service_end) from member_premium where m_id=%s and event_id=%d and service_end>now() and server_id in(0,%d) order by service_end desc limit 1",
                NumberToString(payload->member_id, 0),
                event_id,
                it->server_id);
            if (game_db->exec(true) != true) {
                continue;
            }
            if (!game_db->get_n_rows()) {
                remove_ids.push_back(event_id);
                continue;
            }

            const int rows = game_db->get_n_rows();
            for (int row = 0; row < rows; ++row) {
                if (game_db->fetch() != true) {
                    break;
                }
                int pre_type = 0;
                int service_start = 0;
                int service_end = 0;
                if (game_db->get_int(0, pre_type) != true) {
                    cMyTrace trace(
                        "void DB_Login::GetPremiumInfo(int, SIG_LOGIN_DATA*, DB_Login::vecDNFEventEntry_t&)",
                        13864,
                        5);
                    trace("DB_Login::GetPremiumInfo, get(1) ERROR m_id=%s",
                          NumberToString(payload->member_id,
                                         0));
                    goto done;
                }
                if (game_db->get_int(1, service_start) != true) {
                    cMyTrace trace(
                        "void DB_Login::GetPremiumInfo(int, SIG_LOGIN_DATA*, DB_Login::vecDNFEventEntry_t&)",
                        13870,
                        5);
                    trace("DB_Login::GetPremiumInfo, get(2) ERROR m_id=%s",
                          NumberToString(payload->member_id,
                                         0));
                    goto done;
                }
                if (game_db->get_int(2, service_end) != true) {
                    cMyTrace trace(
                        "void DB_Login::GetPremiumInfo(int, SIG_LOGIN_DATA*, DB_Login::vecDNFEventEntry_t&)",
                        13876,
                        5);
                    trace("DB_Login::GetPremiumInfo, get(3) ERROR m_id=%s",
                          NumberToString(payload->member_id,
                                         0));
                    goto done;
                }

                const int channel_type = G_GameWorld()->GetChannelType();
                if ((channel_type == 7 || channel_type == 14) &&
                    (pre_type == 22 || pre_type == 27)) {
                    continue;
                }
                if (premium_count > 105) {
                    break;
                }
                SIG_LOGIN_DATA::PremiumEntry& premium =
                    payload->premium_entries[premium_count];
                premium.event_id = event_id;
                premium.premium_type = pre_type;
                premium.service_start = service_start;
                premium.service_end = service_end;
                ++premium_count;

                const unsigned short index = payload->premium_event_count;
                payload->premium_event_ids[index] = event_id;
                payload->premium_event_count = index + 1;
            }
        }
    }

    payload->premium_count = premium_count;
    RemovePreminuInfo(payload, remove_ids);

done:
    return;
}

// IDA_EA: 0x84128C4
// IDA_NAME: _ZN8DB_Login17RemovePreminuInfoEP14SIG_LOGIN_DATARSt6vectorIiSaIiEE
// DEMANGLED: DB_Login::RemovePreminuInfo(SIG_LOGIN_DATA *,std::vector<int,std::allocator<int>> &)
bool DB_Login::RemovePreminuInfo(SIG_LOGIN_DATA* payload,
                                 std::vector<int>& event_ids) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }
    for (std::vector<int>::const_iterator it = event_ids.begin();
         it != event_ids.end();
         ++it) {
        db_handle->set_query("deLete from dnf_event_entry where m_id=%s and event_id=%d",
                             NumberToString(payload->member_id, 0),
                             *it);
        db_handle->exec(true);
    }
    return true;
}

// IDA_EA: 0x84129D6
// IDA_NAME: _ZN8DB_Login15_IsPunishedUserEj
// DEMANGLED: DB_Login::_IsPunishedUser(unsigned int)
bool DB_Login::_IsPunishedUser(unsigned int account_id) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!db_handle) {
        return false;
    }
    db_handle->set_query("seLect count(*) from user_ban where m_id=%s and status=1",
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    int count = 0;
    return db_handle->get_int(0, count) == true && count != 0;
}

// IDA_EA: 0x8412AC8
// IDA_NAME: _ZN8DB_Login19_isPunishedHackUserEj
// DEMANGLED: DB_Login::_isPunishedHackUser(unsigned int)
bool DB_Login::_isPunishedHackUser(unsigned int account_id) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect occ_time,period,now_flag from member_punish_hack where m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    int occ_time = 0;
    int period = 0;
    int now_flag = 0;
    if (db_handle->get_int(0, occ_time) != true) return false;
    if (db_handle->get_int(1, period) != true) return false;
    if (db_handle->get_int(2, now_flag) != true) return false;
    CSystemTime* system_time = GlobalData::system_time();
    if (period && system_time &&
        system_time->getCurSec() > occ_time + period * 3600) {
        return false;
    }
    return now_flag != 0;
}

// IDA_EA: 0x8412C52
// IDA_NAME: _ZN8DB_Login10IsPCRoomIPEPc
// DEMANGLED: DB_Login::IsPCRoomIP(char *)
bool DB_Login::IsPCRoomIP(char* ip) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!ip || !*ip || !db_handle) {
        return false;
    }
    char prefix[32];
    memset(prefix, 0, sizeof(prefix));
    strncpy(prefix, ip, sizeof(prefix) - 1);
    char* last_dot = strrchr(prefix, '.');
    if (!last_dot) {
        return false;
    }
    const int end_ip = atoi(last_dot + 1);
    *last_dot = 0;
    db_handle->set_query(
        "seLect count(*) from dnf_pcroom where ip='%s' and start_ip <= %d and end_ip >= %d",
        prefix,
        end_ip,
        end_ip);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    int count = 0;
    return db_handle->get_int(0, count) == true && count != 0;
}

// IDA_EA: 0x8412EB8
// IDA_NAME: _ZN8DB_Login13CreateAccountEP14SIG_LOGIN_DATAP5MySQL
// DEMANGLED: DB_Login::CreateAccount(SIG_LOGIN_DATA *,MySQL *)
bool DB_Login::CreateAccount(SIG_LOGIN_DATA* payload, MySQL* db_handle) {
    if (!payload || !db_handle) {
        return false;
    }
    char* user_id = payload->login_user_id;
    db_handle->set_query("call usp_create_account('%s')", user_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::CreateAccount(SIG_LOGIN_DATA*, MySQL*)",
                              14310,
                              "[Taiwan, Account] Fail create account. id=%s",
                              user_id);
        return false;
    }
    cMyTrace trace("bool DB_Login::CreateAccount(SIG_LOGIN_DATA*, MySQL*)",
                   14314,
                   0);
    trace("[Taiwan, Account] Success create account. id=%s", user_id);
    db_handle->set_query(
        "seLect m_id, passwd,first_ssn, second_ssn,DATE_FORMAT(from_unixtime(reg_date),'%%Y%%m%%d%%H'),user_name,email,hangame_flag,reg_date from member_info where user_id='%s' and hangame_flag=%d",
        user_id,
        static_cast<int>(payload->login_state));
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::CreateAccount(SIG_LOGIN_DATA*, MySQL*)",
                              14340,
                              "[Taiwan, Account] Fail select member_info m_id=%s",
                              user_id);
        return false;
    }
    return true;
}

// IDA_EA: 0x841300E
// IDA_NAME: _ZN8DB_Login16_DecryptUserInfoEPcS0_j
// DEMANGLED: DB_Login::_DecryptUserInfo(char *,char *,unsigned int)
bool DB_Login::_DecryptUserInfo(char* encrypted,
                                char* output,
                                unsigned int account_id) {
    unsigned char rijndael[kCRijndaelSize];
    unsigned char binary[128];
    memset(binary, 0, sizeof(binary));

    new (reinterpret_cast<CRijndael*>(rijndael)) CRijndael();
    reinterpret_cast<CRijndael*>(rijndael)->Initialize(
                         "sbTgiYsfxpfg5fox",
                         32,
                         CRijndael::sm_chain0,
                         16,
                         16,
                         0,
                         0);

    bool result = false;
    if ((strlen(encrypted) >> 1) <= sizeof(binary)) {
        const size_t encrypted_len = strlen(encrypted);
        if (DNFFLib::Hex2Binary(encrypted,
                                binary,
                                static_cast<int>(encrypted_len >> 1)) == 1) {
            const size_t decrypt_len = strlen(encrypted) >> 1;
            const int decrypt_result =
                reinterpret_cast<CRijndael*>(rijndael)->Decrypt(
                                  reinterpret_cast<const char*>(binary),
                                  output,
                                  static_cast<unsigned int>(decrypt_len));
            if (decrypt_result == kCRijndaelSuccess) {
                result = true;
            } else {
                cMyTrace trace(
                    "bool DB_Login::_DecryptUserInfo(char*, char*, memberIdentificationNumber_t)",
                    14368,
                    5);
                trace("DB_Login::_DecryptUserInfo::Decrypt ERROR m_id=%s ret=%u",
                      NumberToString(account_id, 0),
                      decrypt_result);
            }
        } else {
            cMyTrace trace(
                "bool DB_Login::_DecryptUserInfo(char*, char*, memberIdentificationNumber_t)",
                14363,
                5);
            trace("DB_Login::_DecryptUserInfo::Hex2Binary ERROR m_id=%s",
                  NumberToString(account_id, 0));
        }
    } else {
        cMyTrace trace(
            "bool DB_Login::_DecryptUserInfo(char*, char*, memberIdentificationNumber_t)",
            14357,
            5);
        trace("DB_Login::_DecryptUserInfo ERROR m_id=%s",
              NumberToString(account_id, 0));
    }

    reinterpret_cast<CRijndael*>(rijndael)->~CRijndael();
    return result;
}

// IDA_EA: 0x841325E
// IDA_NAME: _ZN8DB_Login14GetAccountInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetAccountInfo(SIG_LOGIN_DATA *)
bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!payload || !db_handle) {
        return false;
    }

    db_handle->set_query(
        "seLect m_id, passwd,first_ssn, second_ssn,DATE_FORMAT(from_unixtime(reg_date),'%%Y%%m%%d%%H'),user_name,email,hangame_flag,reg_date from  member_info  where  user_id='%s' and hangame_flag=%d",
        payload->login_user_id,
        static_cast<int>(payload->login_state));
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14719,
                       5);
        trace("DB_Login::GetAccountInfo, exec() ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (!db_handle->get_n_rows() && CreateAccount(payload, db_handle) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14726,
                       0);
        trace("[Taiwan, Auth] Fail create account. id=%s",
              payload->login_user_id);
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14767,
                       5);
        trace("DB_Login::GetAccountInfo, fetch() ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (db_handle->get_uint(0, payload->member_id) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14773,
                       5);
        trace("DB_Login::GetAccountInfo, get(0) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (db_handle->get_n_rows() != 1) {
        const int row_count = db_handle->get_n_rows();
        const char* account_id =
            NumberToString(payload->member_id, 0);
        db_handle->set_query("upDate member_abnomal set overlab_count=%d where user_id=%s",
                             row_count,
                             account_id);
        if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
            account_id =
                NumberToString(payload->member_id, 0);
            db_handle->set_query(
                "inSert into member_abnomal(user_id, overlab_count) values(%s,%d)",
                account_id,
                row_count);
            db_handle->exec(true);
        }
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14786,
                       5);
        trace("DB_Login::GetAccountInfo, get_n_rows()!=1 ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }

    char password[kMemberInfoPasswordMax];
    memset(password, 0, sizeof(password));
    if (db_handle->get_str(1, password, sizeof(password)) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14792,
                       5);
        trace("DB_Login::GetAccountInfo, get(1) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }

    WongWork::CSimpleSSO::stOutputData_t sso_output;
    if (!GlobalData::s_psimpleSSO ||
        GlobalData::s_psimpleSSO->checkValidPassword(
            payload->member_id,
            password,
            payload->login_password,
            sso_output) != 1) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
            14810,
            "[Taiwan, Auth] Fail query member_login. (mid:%s)",
            NumberToString(payload->member_id, 0));
        return false;
    }

    payload->last_play_time = sso_output.last_play_time;
    payload->account_fail = sso_output.account_fail;
    payload->total_account_fail = sso_output.total_account_fail;
    payload->report_count = sso_output.report_count;
    payload->reliable_flag = sso_output.reliable_flag;
    payload->trade_gold_daily = sso_output.trade_gold_daily;
    payload->dungeon_gain_gold = sso_output.dungeon_gain_gold;
    payload->school_id = sso_output.school_id;
    memset(payload->school_name, 0, sizeof(payload->school_name));
    if (G_GameWorld()->IsSchoolPvPChannel() &&
        payload->school_id) {
        GetSchoolName(payload->school_id, payload->school_name);
        GetBadgeCount(payload);
    }
    payload->tutorial_skipable = sso_output.tutorial_skipable;
    payload->security_card_service = sso_output.security_flag;
    strncpy(payload->sso_login_ip, sso_output.login_ip, 0x10u);
    CSystemTime* system_time = GlobalData::system_time();
    CEnvironment* environment = G_CEnvironment();
    const int cur_sec = system_time ? system_time->getCurSec() : 0;
    if (environment &&
        CheckDailyScheduleTime(environment->get_fatigue_time(),
                               static_cast<unsigned int>(
                                   sso_output.last_play_time),
                               cur_sec)) {
        payload->dungeon_gain_gold = 0;
    }
    payload->event_charac_flag = sso_output.event_charac_flag;

    char first_ssn_cipher[33];
    char second_ssn_cipher[33];
    char first_ssn_text[33];
    char second_ssn_text[33];
    char email_cipher[129];
    char email_text[129];
    memset(first_ssn_cipher, 0, sizeof(first_ssn_cipher));
    memset(second_ssn_cipher, 0, sizeof(second_ssn_cipher));
    memset(first_ssn_text, 0, sizeof(first_ssn_text));
    memset(second_ssn_text, 0, sizeof(second_ssn_text));
    memset(email_cipher, 0, sizeof(email_cipher));
    memset(email_text, 0, sizeof(email_text));

    if (db_handle->get_str(2, first_ssn_cipher, sizeof(first_ssn_cipher)) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14911,
                       5);
        trace("DB_Login::GetAccountInfo, get(2) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    unsigned int first_ssn_number = 0;
    if (first_ssn_cipher[0]) {
        if (_DecryptUserInfo(first_ssn_cipher,
                             first_ssn_text,
                             payload->member_id) != true) {
            cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                           14920,
                           5);
            trace("_DecryptUserInfo(1) ERROR m_id=%s",
                  NumberToString(payload->member_id, 0));
            return false;
        }
        first_ssn_number = static_cast<unsigned int>(atoi(first_ssn_text));
    }

    if (db_handle->get_str(3, second_ssn_cipher, sizeof(second_ssn_cipher)) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14929,
                       5);
        trace("DB_Login::GetAccountInfo, get(3) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    unsigned int second_ssn_number = 0;
    if (second_ssn_cipher[0]) {
        if (_DecryptUserInfo(second_ssn_cipher,
                             second_ssn_text,
                             payload->member_id) != true) {
            cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                           14938,
                           5);
            trace("_DecryptUserInfo(2) ERROR m_id=%s",
                  NumberToString(payload->member_id, 0));
            return false;
        }
        second_ssn_number = static_cast<unsigned int>(atoi(second_ssn_text));
    }

    unsigned int register_date_yyyymmddhh = 0;
    if (db_handle->get_uint(4, register_date_yyyymmddhh) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14961,
                       5);
        trace("DB_Login::GetAccountInfo, get(4) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (db_handle->get_str(5, payload->user_name, 128) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       14995,
                       5);
        trace("DB_Login::GetAccountInfo, get(5) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (db_handle->get_str(6, email_cipher, sizeof(email_cipher)) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       15014,
                       5);
        trace("DB_Login::GetAccountInfo, get(6) ERROR m_id=%s",
              payload->login_user_id);
        return false;
    }
    if (email_cipher[0] &&
        _DecryptUserInfo(email_cipher,
                         email_text,
                         payload->member_id) != true) {
        return false;
    }
    snprintf(payload->email,
             0x7Fu,
             "%s",
             email_text);

    unsigned int hangame_flag = 0;
    if (db_handle->get_uint(7, hangame_flag) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       15040,
                       5);
        trace("DB_Login::GetAccountInfo, get(7) ERROR m_id=%s",
              NumberToString(payload->member_id, 0));
        return false;
    }
    payload->hangame_flag = hangame_flag != 0;

    unsigned int register_date_unix = 0;
    if (db_handle->get_uint(8, register_date_unix) != true) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       15078,
                       5);
        trace("DB_Login::GetAccountInfo, get(9) ERROR m_id=%s",
              NumberToString(payload->member_id, 0));
    }
    payload->register_date_unix = register_date_unix;
    snprintf(payload->member_identity,
             0x10u,
             "%d%d",
             first_ssn_number,
             second_ssn_number);

    const unsigned int birth_year = first_ssn_number / 10000u;
    const unsigned int second_group = second_ssn_number / 1000000u;
    payload->birth_year =
        birth_year + ((second_group <= 2) ? 1900u : 2000u);
    payload->register_day = register_date_yyyymmddhh / 100u;

    struct tm register_tm;
    memset(&register_tm, 0, sizeof(register_tm));
    unsigned int register_date_cursor = register_date_yyyymmddhh;
    register_tm.tm_isdst = -1;
    register_tm.tm_year = static_cast<int>(register_date_cursor / 1000000u) - 1900;
    register_date_cursor %= 1000000u;
    register_tm.tm_mon = static_cast<int>(register_date_cursor / 10000u) - 1;
    register_date_cursor %= 10000u;
    register_tm.tm_mday = static_cast<int>(register_date_cursor / 100u);
    register_date_cursor %= 100u;
    register_tm.tm_hour = static_cast<int>(register_date_cursor);
    register_tm.tm_min = 0;
    register_tm.tm_sec = 0;
    payload->register_time =
        static_cast<unsigned int>(mktime(&register_tm));

    if (Taiwan::GarenaCheckTimeStamp(
            payload->token_timestamp,
            static_cast<unsigned int>(cur_sec),
            sso_output.login_time,
            static_cast<unsigned int>(sso_output.last_play_time))) {
        cMyTrace trace("bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                       15131,
                       0);
        trace("[Taiwan, Auth] Success Token reuse login. (mid:%s, token_time:%u, current_current:%u, login_time:%u, last_play_time:%u)",
              NumberToString(payload->member_id, 0),
              payload->token_timestamp,
              cur_sec,
              sso_output.login_time,
              sso_output.last_play_time);
    } else {
        if (!GetWhiteAccount(payload)) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
                15124,
                "[Taiwan, Auth] Fail Token reuse login. (mid:%s, token_time:%u, current_time:%u, login_time:%u, last_play_time:%u)",
                NumberToString(payload->member_id, 0),
                payload->token_timestamp,
                cur_sec,
                sso_output.login_time,
                sso_output.last_play_time);
            return false;
        }
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_Login::GetAccountInfo(SIG_LOGIN_DATA*)",
            15119,
            "[Taiwan, Auth] White Account Success Token reuse login. (mid:%s, token_time:%u, current_time:%u, login_time:%u, last_play_time:%u)",
            NumberToString(payload->member_id, 0),
            payload->token_timestamp,
            cur_sec,
            sso_output.login_time,
            sso_output.last_play_time);
    }
    return true;
}

// IDA_EA: 0x84143AC
// IDA_NAME: _ZN8DB_Login15GetWhiteAccountEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetWhiteAccount(SIG_LOGIN_DATA *)
bool DB_Login::GetWhiteAccount(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!payload || !db_handle) {
        return false;
    }
    db_handle->set_query("seLect m_id from member_white_account where m_id=%s",
                         NumberToString(payload->member_id, 0));
    return db_handle->exec(true) == true && db_handle->get_n_rows() != 0;
}

// IDA_EA: 0x8414446
// IDA_NAME: _ZN8DB_Login13GetSchoolNameEiPc
// DEMANGLED: DB_Login::GetSchoolName(int,char *)
bool DB_Login::GetSchoolName(int school_id, char* dest) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(16),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!dest || !db_handle) {
        return false;
    }
    db_handle->set_query("seLect school_name from school_info where school_id=%d",
                         school_id);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    return db_handle->get_str(0, dest, 40) == true;
}

// IDA_EA: 0x8414512
// IDA_NAME: _ZN8DB_Login13GetBadgeCountEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetBadgeCount(SIG_LOGIN_DATA *)
bool DB_Login::GetBadgeCount(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(16),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }
    for (int index = 0; index <= 2; ++index) {
        payload->school_badge[index] = 0;
    }
    const int init_time = getDailyInitializeTime(6, 0, 0);
    db_handle->set_query(
        "seLect daily_green_badge, daily_red_badge, daily_blue_badge from school_member where school_id=%d and m_id = %s and  last_play_time >= from_unixtime(%d)",
        payload->school_id,
        NumberToString(payload->member_id, 0),
        init_time);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_short(0, payload->school_badge[0]) != true) return false;
    if (db_handle->get_short(1, payload->school_badge[1]) != true) return false;
    return db_handle->get_short(2, payload->school_badge[2]) == true;
}

// IDA_EA: 0x8414772
// IDA_NAME: _ZN8DB_Login13GetGameDBInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetGameDBInfo(SIG_LOGIN_DATA *)
bool DB_Login::GetGameDBInfo(SIG_LOGIN_DATA* payload) {
    const int charac_view = _getGameDBCharacView(payload);
    if (charac_view == -1) {
        return true;
    }
    if (charac_view == 1) {
        return false;
    }
    if (G_GameWorld()->GetChannelType() != 7) {
        return false;
    }
    const int count = static_cast<int>(payload->charac_view_count);
    for (int index = 0; index < count; ++index) {
        if (payload->charac_view[index].charac_no ==
            payload->current_charac_no) {
            memmove(&payload->charac_view[0],
                    &payload->charac_view[index],
                    sizeof(payload->charac_view[0]));
            if (payload->current_charac_name[0]) {
                strncpy(payload->charac_view[0].identity.character_name,
                        payload->current_charac_name,
                        sizeof(payload->charac_view[0]
                                   .identity.character_name));
            }
            payload->charac_view_count = 1;
            return index == count;
        }
    }
    return true;
}

// IDA_EA: 0x84148A6
// IDA_NAME: _ZN8DB_Login13SaveLoginTimeEjiPc
// DEMANGLED: DB_Login::SaveLoginTime(unsigned int,int,char *)
bool DB_Login::SaveLoginTime(unsigned int account_id, int login_time, char* ip) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "inSert into %s(m_id, login_time, login_ip) values(%s, %d, '%s') on duplicate key upDate login_time=%d, login_ip='%s'",
        "member_login",
        NumberToString(account_id, 0),
        login_time,
        ip ? ip : "",
        login_time,
        ip ? ip : "");
    if (db_handle->exec(true) != true) {
        cMyTrace trace("void DB_Login::SaveLoginTime(memberIdentificationNumber_t, int, char*)",
                       15313,
                       0);
        trace("DB_Login::SaveLoginTime, inSert ERROR m_id=%s, logintime=%d, ip=%s",
              NumberToString(account_id, 0),
              login_time,
              ip ? ip : "");
        return false;
    }
    return false;
}

// IDA_EA: 0x84149CE
// IDA_NAME: _ZN8DB_Login20_getGameDBCharacViewEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA *)
int DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return -1;
    }
    MySQL* db_handle = _getGameDB(DB_HANDLE_2);
    if (!db_handle) {
        return -1;
    }
    if (!db_handle->set_query(
            "seLect info,hash_key,password(info),slot_effect_count,charac_slot_limit from charac_view where m_id=%s",
            NumberToString(payload->member_id, 0))) {
        return -1;
    }
    if (db_handle->exec(true) != true) {
        return -1;
    }
    if (!db_handle->get_n_rows()) {
        return 1;
    }
    if (db_handle->fetch() != true) {
        return -1;
    }

    std::vector<char> original_blob;
    unsigned int raw_size = 0;
    if (!ReadCharacViewCompressedBlob(
            db_handle, 0, &original_blob, &raw_size)) {
        cMyTrace trace(
            "DB_Login::RV_GetGameDBCharacInfo DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA*)",
            15339,
            5);
        trace("DB_Login::_getGameDBCharacView invalid compressed header, m_id(%d)",
              payload->member_id);
        return -1;
    }

    bool upgrade_legacy_blob = false;
    if (raw_size == 0 && original_blob.empty()) {
        memset(payload->charac_view_blob,
               0,
               sizeof(payload->charac_view_blob));
    } else if (raw_size == kNativeCharacViewBlobSize) {
        if (!DecodeCharacViewCompressedBlob(original_blob,
                                            raw_size,
                                            payload->charac_view_blob)) {
            cMyTrace trace(
                "DB_Login::RV_GetGameDBCharacInfo DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA*)",
                15339,
                5);
            trace("DB_Login::_getGameDBCharacView uncompress fail, m_id(%d)",
                  payload->member_id);
            return -1;
        }
    } else if (raw_size == kLegacyCharacViewBlobSize) {
        oldsrv::db::charac_view_compat::LegacyCharacViewEntry legacy_entries[
            kCharacViewCountMax];
        memset(legacy_entries, 0, sizeof(legacy_entries));
        if (!DecodeCharacViewCompressedBlob(
                original_blob,
                raw_size,
                reinterpret_cast<char*>(legacy_entries)) ||
            !oldsrv::db::charac_view_compat::UpgradeLegacyCharacViewBlob(
                reinterpret_cast<const char*>(legacy_entries),
                sizeof(legacy_entries),
                payload->charac_view,
                kCharacViewCountMax)) {
            cMyTrace trace(
                "DB_Login::RV_GetGameDBCharacInfo DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA*)",
                15339,
                5);
            trace("DB_Login::_getGameDBCharacView legacy upgrade fail, m_id(%d)",
                  payload->member_id);
            return -1;
        }
        upgrade_legacy_blob = true;
    } else {
        cMyTrace trace(
            "DB_Login::RV_GetGameDBCharacInfo DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA*)",
            15339,
            5);
        trace("DB_Login::_getGameDBCharacView unsupported raw size(%u), m_id(%d)",
              raw_size,
              payload->member_id);
        return -1;
    }
    char hash_key[264];
    char password_hash[132];
    memset(hash_key, 0, sizeof(hash_key));
    memset(password_hash, 0, sizeof(password_hash));
    if (db_handle->get_binary(1, hash_key, 132) != true) return -1;
    if (db_handle->get_binary(2, password_hash, 132) != true) return -1;
    if (db_handle->get_ubyte(3, payload->charac_slot_effect_count) != true) return -1;
    if (db_handle->get_ubyte(4, payload->charac_slot_limit) != true) return -1;

    // Do not execute the UPDATE until every selected column has been copied:
    // reusing this MySQL handle for the write invalidates the fetched row.
    // A failed write keeps the original legacy blob intact and fails login
    // closed instead of running indefinitely with a half-migrated account.
    if (upgrade_legacy_blob &&
        !RewriteNativeCharacViewBlob(db_handle,
                                     payload->member_id,
                                     payload->charac_view_blob,
                                     original_blob)) {
        cMyTrace trace(
            "DB_Login::RV_GetGameDBCharacInfo DB_Login::_getGameDBCharacView(SIG_LOGIN_DATA*)",
            15339,
            5);
        trace("DB_Login::_getGameDBCharacView native writeback fail, m_id(%d)",
              payload->member_id);
        return -1;
    }

    // OFFICIAL_IDA_EVIDENCE: _getGameDBCharacView @0x84149CE scans the 36
    // records at SIG_LOGIN_DATA +0xE0 and publishes their count at +0xDC
    // (ida_full_restore DB_Login.cpp:2395,2407..2413).  SetCharacInfo
    // @0x8649C6A then consumes exactly that many records as a contiguous
    // prefix.  Preserve the official producer/consumer contract by stably
    // compacting non-empty records before publishing the count.  This is a
    // payload projection only: it neither changes character slots in SQL nor
    // issues an asynchronous per-character query.
    unsigned int compact_count = 0u;
    for (int source_index = 0;
         source_index < kCharacViewCountMax;
         ++source_index) {
        const SIG_LOGIN_DATA::CharacViewEntry& source =
            payload->charac_view[source_index];
        if (!source.charac_no) {
            const bool has_identity_residue =
                source.identity.job != 0u ||
                source.identity.grow_type != 0u ||
                source.identity.level != 0 ||
                source.identity.character_name[0] != 0;
            if (has_identity_residue) {
                LogCharacViewSlot(source_index,
                                  source,
                                  "empty_charac_no_with_identity");
            }
            continue;
        }

        if (compact_count != static_cast<unsigned int>(source_index)) {
            LogCharacViewSlot(source_index, source, "stable_compact");
            payload->charac_view[compact_count] = source;
        }
        ++compact_count;
    }
    if (compact_count < kCharacViewCountMax) {
        memset(&payload->charac_view[compact_count],
               0,
               sizeof(payload->charac_view[0]) *
                   (kCharacViewCountMax - compact_count));
    }
    payload->charac_view_count = compact_count;
    return 0;
}

// IDA_EA: 0x8414C6E
// IDA_NAME: _ZN8DB_Login10SendResultEiiP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::SendResult(int,int,SIG_LOGIN_DATA *)
bool DB_Login::SendResult(int signal, int user_idx, SIG_LOGIN_DATA* payload) {
    // Keep the result trace free of account fields and DB payload contents.
    LogManager::logFormat(1, "login_flow_diag.cpp",
                          "bool DB_Login::SendResult(int, int, SIG_LOGIN_DATA*)",
                          0,
                          "[LOGIN_FLOW][DB_LOGIN_REPLY] signal=%d uid=%d error=%u",
                          signal, user_idx,
                          payload ? static_cast<unsigned int>(payload->fail_code) : 0u);
    Stream* stream = GlobalData::big_stream_pool()->Acquire("DBThread.cpp",
                                                            15433);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload, kSigLoginDataSize);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8414D50
// IDA_NAME: _ZN8DB_Login15UpdateLoginStatEiP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::UpdateLoginStat(int,SIG_LOGIN_DATA *)
bool DB_Login::UpdateLoginStat(int user_idx, SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                        15453);
    CStreamGuard guard(stream, true);
    *guard << kInterUpdateLoginSignal;
    *guard << user_idx;
    SIG_UPDATE_LOGIN* out = guard->GetInBuffer<SIG_UPDATE_LOGIN>();
    if (!out) {
        return false;
    }
    out->m_id = payload->member_id;
    out->server_group = static_cast<int32_t>(payload->server_group);
    out->channel_no = static_cast<int32_t>(payload->game_server_group);
    strncpy(out->login_ip, payload->login_ip, sizeof(out->login_ip));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return true;
}

// IDA_EA: 0x8414E7A
// IDA_NAME: _ZN8DB_Login30GetCheckPickUpRandomOptionItemEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetCheckPickUpRandomOptionItem(SIG_LOGIN_DATA *)
bool DB_Login::GetCheckPickUpRandomOptionItem(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!payload || !db_handle) {
        return false;
    }
    payload->random_option_check_count = 0;
    db_handle->set_query(
        "seLect check_count from check_pick_up_random_option_item where m_id = %s",
        NumberToString(payload->member_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    return db_handle->get_ubyte(0, payload->random_option_check_count) == true;
}

// IDA_EA: 0x8414F5C
// IDA_NAME: _ZN8DB_Login20GetOntimeUserRecvIdxEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetOntimeUserRecvIdx(SIG_LOGIN_DATA *)
bool DB_Login::GetOntimeUserRecvIdx(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(13),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }
    payload->ontime_user_recv_index = 0;
    const unsigned int table_index =
        payload->member_id % 10;
    db_handle->set_query("seLect last_no from event_1112_ontime_user_%u where m_id=%s",
                         table_index,
                         NumberToString(payload->member_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) return false;
        return db_handle->get_int(0, payload->ontime_user_recv_index) == true;
    }
    payload->ontime_user_recv_index = 1;
    db_handle->set_query(
        "inSert into event_1112_ontime_user_%u(m_id,last_no) values (%s,%u )",
        table_index,
        NumberToString(payload->member_id, 0),
        payload->ontime_user_recv_index);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84150E8
// IDA_NAME: _ZN8DB_Login19GetUserFatigueCountEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetUserFatigueCount(SIG_LOGIN_DATA *)
bool DB_Login::GetUserFatigueCount(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    payload->fatigue_count = 0;
    payload->fatigue_occurrence_day = 0;
    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    if (!event_script_mng ||
        event_script_mng->is_eventing(0x56) != 1) {
        return false;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    const std::string* table_name =
        event_script_mng->get_db_table_name(0x56);
    if (!table_name) {
        return false;
    }

    const int today_day = event_script_mng->get_today_time_day();
    db_handle->set_query(
        "seLect fatigue from %s where m_id=%s and occ_day = %d",
        ::toTString(*table_name),
        NumberToString(payload->member_id, 0),
        today_day);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() > 0) {
        short fatigue = 0;
        if (db_handle->fetch() != true) return false;
        if (db_handle->get_short(0, fatigue) != true) return false;
        payload->fatigue_count = static_cast<unsigned short>(fatigue);
        payload->fatigue_occurrence_day =
            static_cast<unsigned char>(today_day);
    }
    return true;
}

// IDA_EA: 0x84152B0
// IDA_NAME: _ZN8DB_Login25GetUserInGameEventHistoryEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetUserInGameEventHistory(SIG_LOGIN_DATA *)
bool DB_Login::GetUserInGameEventHistory(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!payload || !db_handle) {
        return false;
    }
    memset(payload->in_game_event_history,
           0,
           sizeof(payload->in_game_event_history));
    db_handle->set_query(
        "seLect type , history_1 ,history_2 from event_ingame_history where m_id=%s order by type desc",
        NumberToString(payload->member_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    const int rows = db_handle->get_n_rows();
    for (int row = 0; row < rows && row <= 29; ++row) {
        SIG_LOGIN_DATA::EventHistoryEntry& item =
            payload->in_game_event_history[row];
        if (db_handle->fetch() != true) {
            break;
        }
        if (db_handle->get_int(0, item.type) != true) break;
        if (db_handle->get_int(1, item.history_1) != true) break;
        if (db_handle->get_int(2, item.history_2) != true) break;
    }
    return true;
}

// IDA_EA: 0x8415472
// IDA_NAME: _ZN8DB_Login12CheckLoginIPEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::CheckLoginIP(SIG_LOGIN_DATA *)
bool DB_Login::CheckLoginIP(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    payload->login_ip_changed = 0;
    char current_ip[16];
    char previous_ip[16];
    memset(current_ip, 0, sizeof(current_ip));
    memset(previous_ip, 0, sizeof(previous_ip));
    strncpy(current_ip, payload->login_ip, sizeof(current_ip) - 1);
    strncpy(previous_ip, payload->previous_login_ip, sizeof(previous_ip) - 1);
    char* dot = strrchr(current_ip, '.');
    if (dot) {
        *dot = 0;
    }
    dot = strrchr(previous_ip, '.');
    if (dot) {
        *dot = 0;
    }
    if (!strcmp(current_ip, previous_ip)) {
        return true;
    }
    if (payload->previous_login_ip[0]) {
        payload->login_ip_changed = 1;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query("upDate login_common set login_ip='%s' where m_id=%u",
                         payload->login_ip,
                         payload->member_id);
    if (db_handle->exec(true) == true) {
        return true;
    }
    LogManager::logFormat(1,
                          "DBThread.cpp",
                          "bool DB_Login::CheckLoginIP(SIG_LOGIN_DATA*)",
                          15971,
                          "LOGIN_IP : IP Address UDATE DB ERROR!!");
    return false;
}

// IDA_EA: 0x8415618
// IDA_NAME: _ZN8DB_Login22GetMercenarySystemInfoEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetMercenarySystemInfo(SIG_LOGIN_DATA *)
bool DB_Login::GetMercenarySystemInfo(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    MySQL* bonus_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    MySQL* item_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!bonus_db || !item_db) {
        return false;
    }

    payload->mercenary_count = 0;
    CDataManager* data_manager = G_CDataManager();
    const short minimum_level =
        data_manager
            ? static_cast<short>(
                  data_manager->get_mercenary_system_info()->default_count)
            : 0;

    int mercenary_count = 0;
    const int charac_count = static_cast<int>(payload->charac_view_count);
    for (int index = 0;
         index < charac_count && index <= 35 && mercenary_count <= 35;
         ++index) {
        const SIG_LOGIN_DATA::CharacViewEntry& charac_view =
            payload->charac_view[index];
        const short level = charac_view.identity.level;
        if (level < minimum_level) {
            continue;
        }

        const unsigned int charac_no = charac_view.charac_no;
        CHARAC_LOAD_MERCENARY* mercenary =
            &payload->mercenaries[mercenary_count];
        bonus_db->set_query(
            "seLect mercenary_start_time,mercenary_finish_time,mercenary_area,mercenary_period from charac_link_bonus where charac_no=%u",
            charac_no);
        if (bonus_db->exec(true) != true) {
            return false;
        }

        mercenary->Reset();
        strncpy(mercenary->name,
                charac_view.identity.character_name,
                sizeof(mercenary->name));
        mercenary->charac_no = charac_no;
        mercenary->level = static_cast<uint16_t>(level);

        item_db->set_query(
            "seLect slot,it_id from user_items where charac_no=%u and expire_date>now() and stat=0 order by slot limit 8",
            charac_no);
        if (item_db->exec(true) != true) {
            return false;
        }
        const int item_rows = item_db->get_n_rows();
        if (item_rows == 8) {
            for (int row = 0; row < item_rows; ++row) {
                int slot = 0;
                int item_id = 0;
                if (item_db->fetch() != true) {
                    cMyTrace trace(
                        "bool DB_Login::GetMercenarySystemInfo(SIG_LOGIN_DATA*)",
                        16034,
                        5);
                    trace("DB_Login::GetMercenarySystemInfo %d ERROR Charac_no=%u",
                          162,
                          charac_no);
                    break;
                }
                if (item_db->get_int(0, slot) != true) {
                    cMyTrace trace(
                        "bool DB_Login::GetMercenarySystemInfo(SIG_LOGIN_DATA*)",
                        16040,
                        5);
                    trace("DB_Login::GetMercenarySystemInfo %d ERROR Charac_no=%u",
                          168,
                          charac_no);
                    return false;
                }
                if (item_db->get_int(1, item_id) != true) {
                    cMyTrace trace(
                        "bool DB_Login::GetMercenarySystemInfo(SIG_LOGIN_DATA*)",
                        16046,
                        5);
                    trace("DB_Login::GetMercenarySystemInfo %d ERROR charac_no=%u",
                          174,
                          charac_no);
                    return false;
                }
                if (slot != row) {
                    mercenary->avatar_bonus_index = 0;
                    break;
                }
                mercenary->avatar_item_id[row] = item_id;
            }
        } else {
            mercenary->avatar_bonus_index = 0;
        }

        if (bonus_db->get_n_rows()) {
            if (bonus_db->fetch() == true &&
                bonus_db->get_int(0, mercenary->start_time) == true &&
                bonus_db->get_int(1, mercenary->end_time) == true &&
                bonus_db->get_ubyte(2, mercenary->competition_area) == true &&
                bonus_db->get_ubyte(3, mercenary->competition_period) == true) {
                ++mercenary_count;
                ++payload->mercenary_count;
            }
        } else {
            bonus_db->set_query(
                "inSert into charac_link_bonus (charac_no,mercenary_start_time,mercenary_finish_time,mercenary_area,mercenary_period) values(%u,0,0,-1,-1)",
                charac_no);
            if (bonus_db->exec(true) == true) {
                mercenary->start_time = 0;
                mercenary->end_time = 0;
                mercenary->competition_area = 0xffu;
                mercenary->competition_period = 0xffu;
                ++mercenary_count;
                ++payload->mercenary_count;
            }
        }
    }
    return true;
}

// IDA_EA: 0x8447EA6
// IDA_NAME: _ZN8DB_Login20GetGrowthWeaponEventEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetGrowthWeaponEvent(SIG_LOGIN_DATA *)
bool DB_Login::GetGrowthWeaponEvent(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(100)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return true;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect server_id, charac_no, growthweapon_id, timepiece, infinityweapon_id from event_1207_growthweapon where m_id = %d",
        payload->member_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::GetGrowthWeaponEvent(SIG_LOGIN_DATA*)",
                              46215,
                              "LoadGrowthWeaponEvent Query Error : m_id(%d)",
                              payload->member_id);
        return false;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) return false;
        payload->growth_weapon_active = 1;
        if (db_handle->get_short(0, payload->growth_weapon_server_id) != true) return false;
        if (db_handle->get_int(1, payload->growth_weapon_charac_no) != true) return false;
        if (db_handle->get_int(2, payload->growth_weapon_id) != true) return false;
        if (db_handle->get_short(3, payload->growth_weapon_timepiece) != true) return false;
        if (db_handle->get_int(4, payload->growth_weapon_infinity_id) != true) return false;
    } else {
        payload->growth_weapon_active = 0;
        payload->growth_weapon_server_id = 0;
        payload->growth_weapon_charac_no = 0;
        payload->growth_weapon_id = 0;
        payload->growth_weapon_timepiece = 0;
        payload->growth_weapon_infinity_id = 0;
    }
    return true;
}

// IDA_EA: 0x8448102
// IDA_NAME: _ZN8DB_Login22GetGrowthCreatureEventEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetGrowthCreatureEvent(SIG_LOGIN_DATA *)
bool DB_Login::GetGrowthCreatureEvent(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(103)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return true;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect server_id, charac_no, job_type, charac_name from event_1208_growthcreature where m_id = %d",
        payload->member_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::GetGrowthCreatureEvent(SIG_LOGIN_DATA*)",
                              46265,
                              "LoadGrowthCreatureEvent Query Error : m_id(%d)",
                              payload->member_id);
        return false;
    }
    if (db_handle->get_n_rows()) {
        const int rows = db_handle->get_n_rows();
        for (int row = 0; row < rows; ++row) {
            if (db_handle->fetch() != true) return false;
            if (row > 0xA) return false;
            SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& entry =
                payload->growth_creature_event.entries[row];
            if (db_handle->get_short(
                    0, reinterpret_cast<short&>(entry.job)) != true) {
                return false;
            }
            if (db_handle->get_int(1, entry.charac_no) != true) return false;
            if (db_handle->get_short(
                    2, reinterpret_cast<int&>(entry.creature_id)) != true) {
                return false;
            }
            if (db_handle->get_str(3,
                                   reinterpret_cast<char*>(entry.creature_name),
                                   29) != true) {
                return false;
            }
        }
    } else {
        memset(payload->growth_creature_event.entries,
               0,
               sizeof(payload->growth_creature_event.entries));
    }
    return true;
}

// IDA_EA: 0x844834E
// IDA_NAME: _ZN8DB_Login34GetAccountCharacterLevelUpOnceGiftEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetAccountCharacterLevelUpOnceGift(SIG_LOGIN_DATA *)
bool DB_Login::GetAccountCharacterLevelUpOnceGift(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(104)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return true;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect job_type, level from event_1208_level_achieve where m_id = %d",
        payload->member_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DB_Login::GetAccountCharacterLevelUpOnceGift(SIG_LOGIN_DATA*)",
                              46312,
                              "LoadAccountCharacterLevelUpOnceGift Query Error : m_id(%d)",
                              payload->member_id);
        return false;
    }
    if (db_handle->get_n_rows()) {
        const int rows = db_handle->get_n_rows();
        for (int row = 0; row < rows; ++row) {
            if (db_handle->fetch() != true) return false;
            if (row > 3) return false;
            SIG_LOGIN_DATA::AccountLevelGiftEntry& entry =
                payload->account_level_gifts[row];
            if (db_handle->get_short(0, entry.job_type) != true) return false;
            if (db_handle->get_int(1, entry.level) != true) return false;
        }
    } else {
        memset(payload->account_level_gifts,
               0,
               sizeof(payload->account_level_gifts));
    }
    return true;
}

// IDA_EA: 0x8448BEE
// IDA_NAME: _ZN8DB_Login13GetBingoEventEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::GetBingoEvent(SIG_LOGIN_DATA *)
bool DB_Login::GetBingoEvent(SIG_LOGIN_DATA* payload) {
    if (!payload) {
        return false;
    }
    payload->bingo_reward = 0;
    payload->bingo_board = 0;
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(106)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return true;
    }
    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    CEnvironment* environment = G_CEnvironment();
    CSystemTime* system_time = GlobalData::system_time();
    if (!db_handle || !environment || !system_time) {
        return false;
    }
    const time_t reset_time =
        GetCurrentResetBaseTime(system_time->getCurSec(),
                                environment->get_fatigue_time());
    db_handle->set_query(
        "seLect board,reward from event_1208_bingo where m_id = %s and occ_date > from_unixtime(%u)",
        NumberToString(payload->member_id, 0),
        static_cast<unsigned int>(reset_time));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_Login::GetBingoEvent(SIG_LOGIN_DATA*)",
                       46875,
                       5);
        trace("DB_Login::GetBingoEvent, fetch() ERROR m_id=%s",
              NumberToString(payload->member_id, 0));
        return false;
    }
    int column_index = 1;
    if (db_handle->get_int(0, payload->bingo_board) != true) {
        cMyTrace trace("bool DB_Login::GetBingoEvent(SIG_LOGIN_DATA*)",
                       46883,
                       5);
        trace("DB_Login::GetBingoEvent, get(%d) ERROR m_id=%s",
              column_index,
              NumberToString(payload->member_id, 0));
        return false;
    }
    const int reward_column = column_index++;
    if (db_handle->get_byte(reward_column, payload->bingo_reward) != true) {
        cMyTrace trace("bool DB_Login::GetBingoEvent(SIG_LOGIN_DATA*)",
                       46889,
                       5);
        trace("DB_Login::GetBingoEvent, get(%d) ERROR m_id=%s",
              column_index,
              NumberToString(payload->member_id, 0));
        return false;
    }
    return false;
}

// IDA_EA: 0x844B982
// IDA_NAME: _ZN8DB_Login30IsThisIPLoginPermissionGrantedEP14SIG_LOGIN_DATA
// DEMANGLED: DB_Login::IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA *)
bool DB_Login::IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA* payload) {
    MySQL* db_handle = _GetDNFAccountDB();
    if (!payload || !db_handle) {
        return false;
    }
    unsigned int count = 0;
    payload->ip_permission_granted = 0;
    db_handle->set_query("seLect count(*) from geo_allow where allow_ip = '%s'",
                         payload->login_ip);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_Login::IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA*)",
                       48201,
                       5);
        trace("[QUERY_ERROR] (select from geo_allow)");
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true ||
        db_handle->get_uint(0, count) != true) {
        cMyTrace trace("bool DB_Login::IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA*)",
                       48210,
                       5);
        trace("[QUERY_ERROR] (fetch get_uint)");
        return false;
    }
    if (count) {
        payload->ip_permission_granted = 1;
    }
    db_handle->set_query(
        "insert into member_join_info(m_id, reg_date, ip) values(%d, now(), '%s')",
        payload->member_id,
        payload->login_ip);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_Login::IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA*)",
                       48227,
                       5);
        trace("[QUERY_ERROR] (insert into member_join_info)");
        return false;
    }
    return true;
}
