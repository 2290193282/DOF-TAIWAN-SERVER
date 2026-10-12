#include "old_data/db/account/db_update_logout.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstdio>




// IDA_EA: 0x84183D8
// IDA_NAME: _ZN15DB_UpdateLogoutC2Ev
// DEMANGLED: DB_UpdateLogout::DB_UpdateLogout(void)
DB_UpdateLogout::DB_UpdateLogout() {}

// IDA_EA: 0x84183F4
// IDA_NAME: _ZN15DB_UpdateLogoutD2Ev
// DEMANGLED: DB_UpdateLogout::~DB_UpdateLogout()
// IDA_EA: 0x8418424
// IDA_NAME: _ZN15DB_UpdateLogoutD0Ev
// DEMANGLED: DB_UpdateLogout::~DB_UpdateLogout() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateLogout::~DB_UpdateLogout() {}

// IDA_EA: 0x8418442
// IDA_NAME: _ZN15DB_UpdateLogout31UpdateAccountMemberBonusFatigueEjs
// DEMANGLED: DB_UpdateLogout::UpdateAccountMemberBonusFatigue(unsigned int,short)
int DB_UpdateLogout::UpdateAccountMemberBonusFatigue(unsigned int account_id,
                                                     short bonus_fatigue) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    char account_id_text[32];
    std::snprintf(account_id_text, sizeof(account_id_text), "%u", account_id);
    db_handle->set_query(
        "upDate login_common set member_bonus_fatigue=%d where m_id=%s",
        bonus_fatigue,
        account_id_text);

    int should_insert = 1;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount() != 0) {
            should_insert = 0;
        }
    }

    if (should_insert) {
        std::snprintf(account_id_text, sizeof(account_id_text), "%u",
                      account_id);
        db_handle->set_query(
            "inSert into login_common(m_id,member_bonus_fatigue,update_time) values(%s,%d,unix_timestamp(now()))",
            account_id_text,
            bonus_fatigue);
        return (db_handle->exec(true) ? 1 : 0) ^ 1;
    }
    return should_insert;
}

// IDA_EA: 0x841852E
// IDA_NAME: _ZN15DB_UpdateLogout10SendResultEiiP17SIG_UPDATE_LOGOUT
// DEMANGLED: DB_UpdateLogout::SendResult(int,int,SIG_UPDATE_LOGOUT *)
bool DB_UpdateLogout::SendResult(int signal, int user_idx,
                                 SIG_UPDATE_LOGOUT* logout) {
    (void)user_idx;

    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 0x48E2);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << -1;
    guard->put_binary(logout, sizeof(*logout));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8418612
// IDA_NAME: _ZN15DB_UpdateLogout8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateLogout::dispatch(int,int,Stream *)
bool DB_UpdateLogout::dispatch(int signal, int user_idx, Stream* stream) {
    SIG_UPDATE_LOGOUT logout;
    if (stream->get_binary(&logout, sizeof(logout)) != 1) {
        return false;
    }

    logout.server_id = 0;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        logout.server_id = GlobalData::s_db_mgr->GetServerType(user_idx);
    } else {
        logout.server_id = G_CEnvironment()->get_server_group();
    }

    GlobalData::s_db_mgr->removeUserDBInfo(user_idx);
    if (UpdateLogout(&logout) != true) {
        char account_id_text[32];
        std::snprintf(account_id_text, sizeof(account_id_text), "%u",
                      logout.m_id);
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_UpdateLogout::dispatch(int, int, Stream*)",
            0x4939,
            "UpdateLogout() fail - m_id[%s]\n",
            account_id_text);
        return false;
    }

    if (logout.write_member_play_info) {
        MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
        char account_id_text[32];
        std::snprintf(account_id_text, sizeof(account_id_text), "%u",
                      logout.m_id);

        if (logout.pcbang_flag) {
            db_handle->set_query(
                "upDate member_play_info set play_time=play_time+%d,play_count=play_count+1,trade_cnt=trade_cnt+%d, exp=exp+%u, used_fatigue=used_fatigue+%u, ting_count=ting_count+%d, ip='%s', last_play_time=%u, pcbang_flag=%d, end_ip='%s', mac_addr='%s', server_id = %d where occ_date=cast(now() as date) and m_id=%s",
                logout.play_time,
                logout.trade_count,
                logout.exp,
                static_cast<unsigned int>(logout.used_fatigue),
                logout.ting_flag == 0,
                logout.ip,
                logout.last_play_time,
                static_cast<int>(logout.pcbang_flag),
                logout.end_ip,
                logout.mac_addr,
                logout.server_id,
                account_id_text);
        } else {
            db_handle->set_query(
                "upDate member_play_info set play_time=play_time+%d,play_count=play_count+1,trade_cnt=trade_cnt+%d, exp=exp+%u, used_fatigue=used_fatigue+%u, ting_count=ting_count+%d, ip='%s', last_play_time=%u, end_ip='%s', mac_addr='%s', server_id = %d where occ_date=cast(now() as date) and m_id=%s",
                logout.play_time,
                logout.trade_count,
                logout.exp,
                static_cast<unsigned int>(logout.used_fatigue),
                logout.ting_flag == 0,
                logout.ip,
                logout.last_play_time,
                logout.end_ip,
                logout.mac_addr,
                logout.server_id,
                account_id_text);
        }

        if (logout.m_id) {
            bool should_insert = true;
            if (db_handle->exec(true) == true) {
                if (db_handle->getAffectedRowCount() != 0) {
                    should_insert = false;
                }
            }

            if (should_insert) {
                std::snprintf(account_id_text, sizeof(account_id_text), "%u",
                              logout.m_id);
                db_handle->set_query(
                    "inSert into member_play_info(occ_date,m_id,play_time,play_count,trade_cnt,exp,used_fatigue,ting_count, ip, last_play_time, pcbang_flag, end_ip, mac_addr, server_id) values(cast(now() as date),%s,%d,1,%d,%u,%u,%d,'%s', %u, %d, '%s', '%s',%d)",
                    account_id_text,
                    logout.play_time,
                    logout.trade_count,
                    logout.exp,
                    static_cast<unsigned int>(logout.used_fatigue),
                    logout.ting_flag == 0,
                    logout.ip,
                    logout.last_play_time,
                    static_cast<int>(logout.pcbang_flag),
                    logout.end_ip,
                    logout.mac_addr,
                    logout.server_id);
                if (db_handle->exec(true) != true) {
                    return false;
                }
            }
        }
    }

    if (logout.save_member_bonus) {
        UpdateAccountMemberBonusFatigue(logout.m_id,
                                        logout.member_bonus_fatigue);
    }
    return SendResult(signal, user_idx, &logout);
}

// IDA_EA: 0x8418A96
// IDA_NAME: _ZN15DB_UpdateLogout12UpdateLogoutEP17SIG_UPDATE_LOGOUT
// DEMANGLED: DB_UpdateLogout::UpdateLogout(SIG_UPDATE_LOGOUT *)
bool DB_UpdateLogout::UpdateLogout(SIG_UPDATE_LOGOUT* logout) {
    if (!logout->m_id) {
        return true;
    }

    int login_account_table = 0;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        login_account_table = logout->server_id;
    } else {
        login_account_table = G_CEnvironment()->get_server_group();
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    char account_id_text[32];
    std::snprintf(account_id_text, sizeof(account_id_text), "%u",
                  logout->m_id);
    db_handle->set_query(
        "upDate login_account_%d set m_channel_no=%d,login_status=%d,last_login_date=now() where m_id=%s",
        login_account_table,
        logout->channel_no,
        0,
        account_id_text);
    return db_handle->exec(true) == true;
}
