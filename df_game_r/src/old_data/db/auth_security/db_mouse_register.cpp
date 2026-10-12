#include "old_data/db/auth_security/db_mouse_register.h"

#include "old_data/db/auth_security/db_check_secu_reward.h"

#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>
#include <time.h>



class CRijndael;

extern char* encryptAES_(CRijndael* crypt, char* src, char* dest)
    __asm__("_Z11encryptAES_P9CRijndaelPcS1_");
namespace {

enum {
    kMouseRegisterInterSignal = 163,
    kGoblinPadAdviceChangePwdSignal = 581,
    kGoblinPadAdviceClearPwdSignal = 582,
};

}  // namespace

// IDA_EA: 0x8422938
// IDA_NAME: _ZN16DB_MouseRegister30_getDBHandleForDevideAccountDBEv
// DEMANGLED: DB_MouseRegister::_getDBHandleForDevideAccountDB(void)
MySQL* DB_MouseRegister::_getDBHandleForDevideAccountDB() {
    return GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
}

// IDA_EA: 0x8422964
// IDA_NAME: _ZN16DB_MouseRegister8dispatchEiiP6Stream
// DEMANGLED: DB_MouseRegister::dispatch(int,int,Stream *)
bool DB_MouseRegister::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)_getDBHandleForDevideAccountDB();
    SIG_MOUSE_REGISTER* payload =
        stream->GetOutBuffer<SIG_MOUSE_REGISTER>();

    char ip[16];
    memset(ip, 0, sizeof(ip));
    strncpy(ip, payload->ip, sizeof(ip));

    char plain_password[36];
    memset(plain_password, 0, sizeof(plain_password));
    strncpy(plain_password, payload->requested_password,
            sizeof(payload->requested_password));

    char encrypted_password[36];
    memset(encrypted_password, 0, sizeof(encrypted_password));
    encryptAES_(GlobalData::s_pcryptRijndael_MousePassword_,
                plain_password,
                encrypted_password);

    if (!strncmp(payload->encrypted_password,
                 encrypted_password,
                 0x24u)) {
        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 23385);
        CStreamGuard guard(out_stream, true);
        *guard << kGoblinPadAdviceClearPwdSignal;
        *guard << user_idx;
        SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD* out_payload =
            guard->GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>();
        if (!out_payload) {
            return false;
        }
        out_payload->advice_change_password = 0;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
        return false;
    }

    int result = 0;
    switch (payload->request_kind) {
        case 2:
            result = _modifyPassword(payload->account_id,
                                     encrypted_password,
                                     ip,
                                     user_idx,
                                     payload->advice_change_password != 0);
            if (!result) {
                memcpy(payload->encrypted_password, encrypted_password,
                       0x24u);
                payload->fail_count = 0;
                payload->cancel_count = 0;
            }
            break;
        case 3:
            result = _unregistPassword(payload->account_id, ip);
            if (!result) {
                memset(payload->encrypted_password, 0, 0x24u);
            }
            break;
        case 1:
            result = _registNewPassword(payload->account_id,
                                        encrypted_password,
                                        ip,
                                        user_idx);
            if (!result) {
                payload->activated = 1;
                payload->fail_count = 0;
                payload->cancel_count = 0;
                memcpy(payload->encrypted_password, encrypted_password,
                       0x24u);
            }
            break;
        default:
            break;
    }
    _sendResult(payload->account_id,
                result,
                payload->request_kind,
                user_idx,
                payload);
    return true;
}

// IDA_EA: 0x8422C8A
// IDA_NAME: _ZN16DB_MouseRegister13checkPasswordEjPKcbRiS2_RbPcS4_
// DEMANGLED: DB_MouseRegister::checkPassword(unsigned int,char const*,bool,int &,int &,bool &,char *,char *)
int DB_MouseRegister::checkPassword(unsigned int account_id,
                                    const char* password,
                                    bool fail_lock_check,
                                    int& fail_count,
                                    int& cancel_count,
                                    bool& password_found,
                                    char* mousepass,
                                    char* encrypted_password) {
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 1;
    }

    password_found = false;
    cancel_count = 0;
    memset(mousepass, 0, 0x24u);
    db_handle->set_query(
        "seLect mousepass,fail_cnt,cancel_cnt, version_info from member_mousepass where m_id=%s and enable_flag = '1'",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return 1;
    }
    if (!db_handle->get_n_rows()) {
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 1;
    }
    if (db_handle->get_str(0, mousepass, 36) != true) {
        return 1;
    }
    if (db_handle->get_int(1, fail_count) != true) {
        return 1;
    }
    if (db_handle->get_int(2, cancel_count) != true) {
        return 1;
    }
    if (fail_lock_check && fail_count > 2) {
        return 73;
    }

    char version_info = 0;
    if (db_handle->get_byte(3, version_info) != true) {
        return 1;
    }
    if (version_info == 1) {
        return 166;
    }

    char plain_password[36];
    memset(plain_password, 0, sizeof(plain_password));
    strncpy(plain_password, password, 0x24u);
    encryptAES_(GlobalData::s_pcryptRijndael_MousePassword_,
                plain_password,
                encrypted_password);
    password_found = true;
    return 0;
}

// IDA_EA: 0x8422E6A
// IDA_NAME: _ZN16DB_MouseRegister18convertPassPadInfoEjPcRiRcS1_RbS0_S3_S0_RlS3_
// DEMANGLED: DB_MouseRegister::convertPassPadInfo(unsigned int,char *,int &,char &,int &,bool &,char *,bool &,char *,long &,bool &)
int DB_MouseRegister::convertPassPadInfo(unsigned int account_id,
                                         char* password,
                                         int& fail_count,
                                         char& passpad_result,
                                         int& cancel_count,
                                         bool& passpad_enabled,
                                         char* mousepass,
                                         bool& validity_flag,
                                         char* ip,
                                         long& validity_time,
                                         bool& month_old_flag) {
    passpad_enabled = false;
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 1;
    }

    db_handle->set_query(
        "seLect mousepass, fail_cnt, cancel_cnt, version_info, validity_time>unix_timestamp(now()), adddate(now(),interval -1 month) >= occ_time from member_mousepass where m_id=%s and enable_flag = '1'",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return 1;
    }
    if (!db_handle->get_n_rows()) {
        passpad_result = 0;
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 1;
    }
    if (db_handle->get_str(0, mousepass, 36) != true) {
        return 1;
    }
    if (db_handle->get_int(1, fail_count) != true) {
        return 1;
    }
    if (db_handle->get_int(2, cancel_count) != true) {
        return 1;
    }

    char version_info = 0;
    if (db_handle->get_byte(3, version_info) != true) {
        return 1;
    }
    uint32_t valid_time = 0;
    validity_time = 0;
    if (db_handle->get_uint(4, valid_time) != true) {
        return 1;
    }
    validity_time = valid_time;

    char month_old = 0;
    if (db_handle->get_byte(5, month_old) != true) {
        return 1;
    }
    month_old_flag = month_old != 0;

    validity_flag = false;
    if (validity_time) {
        validity_flag = true;
    }
    if (version_info == 2) {
        passpad_enabled = true;
        passpad_result = 2;
        return 0;
    }

    passpad_result = 1;
    char plain_password[36];
    char encrypted_password[36];
    memset(plain_password, 0, sizeof(plain_password));
    memset(encrypted_password, 0, sizeof(encrypted_password));
    strncpy(plain_password, password, 0x24u);
    encryptAES_(GlobalData::s_pcryptRijndael_MousePassword_,
                plain_password,
                encrypted_password);
    if (!strncmp(mousepass, encrypted_password, 0x24u)) {
        fail_count = 0;
        cancel_count = 0;
        if (_updateHistory(account_id, 7, ip) == true) {
            db_handle->set_query(
                "upDate member_mousepass set enable_flag = '0' where m_id=%s",
                NumberToString(account_id, 0));
            if (db_handle->exec(true) != true) {
                return 1;
            }
            if (!db_handle->getAffectedRowCount()) {
                return 1;
            }
            passpad_enabled = false;
            return 0;
        }
        return 1;
    }

    ++fail_count;
    _updateFailCnt(account_id, fail_count, ip);
    if (fail_count <= 9) {
        return 72;
    }
    return 73;
}

// IDA_EA: 0x84231B6
// IDA_NAME: _ZN16DB_MouseRegister14_updateFailCntEjiPc
// DEMANGLED: DB_MouseRegister::_updateFailCnt(unsigned int,int,char *)
int DB_MouseRegister::_updateFailCnt(unsigned int account_id,
                                     int fail_count,
                                     char* ip) {
    (void)ip;
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 1;
    }
    db_handle->set_query(
        "upDate member_mousepass set fail_cnt=%d where m_id=%s",
        fail_count,
        NumberToString(account_id, 0));
    db_handle->exec(true);

    MySQL* account_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!account_db) {
        return 1;
    }
    account_db->set_query(
        "upDate member_security_grade set goblin_fail_cnt=%d where m_id=%s",
        fail_count,
        NumberToString(account_id, 0));
    int result = 1;
    if (account_db->exec(true) == true && account_db->getAffectedRowCount()) {
        result = 0;
    }
    if (result) {
        account_db->set_query(
            "inSert into member_security_grade(m_id, goblin_fail_cnt) values(%s, %d)",
            NumberToString(account_id, 0),
            fail_count);
        return account_db->exec(true) == true ? 0 : 1;
    }
    return result;
}

// IDA_EA: 0x84232E0
// IDA_NAME: _ZN16DB_MouseRegister18_registNewPasswordEjPKcPci
// DEMANGLED: DB_MouseRegister::_registNewPassword(unsigned int,char const*,char *,int)
int DB_MouseRegister::_registNewPassword(unsigned int account_id,
                                         const char* encrypted_password,
                                         char* ip,
                                         int user_idx) {
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 1;
    }
    db_handle->set_query(
        "seLect enable_flag from member_mousepass where m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return 1;
    }
    if (db_handle->get_n_rows()) {
        db_handle->set_query(
            "upDate member_mousepass set mousepass = '%s', occ_time = now() ,version_info = '2', validity_time=unix_timestamp(now()), enable_flag = '1', reward_time = 0 where m_id = %s",
            encrypted_password,
            NumberToString(account_id, 0));
    } else {
        db_handle->set_query(
            "inSert into member_mousepass(m_id,mousepass,occ_time,version_info, validity_time, reward_time,enable_flag) values(%s,'%s',now(),'2', unix_timestamp(now()), 0,'1')",
            NumberToString(account_id, 0),
            encrypted_password);
    }
    if (db_handle->exec(true) == true) {
        DB_CancelRestingUserRestrict::makeRequest(account_id, user_idx);
        if (_updateHistory(account_id, 8, ip) == true) {
            MySQL* account_db =
                GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0))
                                    : 0;
            if (!account_db) {
                return 1;
            }
            account_db->set_query(
                "upDate member_security_grade set goblin_pass_mod=now() where m_id=%s",
                NumberToString(account_id, 0));
            bool need_insert = true;
            if (account_db->exec(true) == true &&
                account_db->getAffectedRowCount()) {
                need_insert = false;
            }
            if (!need_insert) {
                DB_CheckSecuReward::makeRequest(user_idx, account_id, 1);
                return 0;
            }
            account_db->set_query(
                "inSert into member_security_grade(m_id, goblin_pass_mod) values(%s, now())",
                NumberToString(account_id, 0));
            if (account_db->exec(true) == true) {
                DB_CheckSecuReward::makeRequest(user_idx, account_id, 1);
                return 0;
            }
            return 1;
        }
        return 1;
    }
    if (db_handle->getDBError() == 1062) {
        return 18;
    }
    (void)user_idx;
    return 1;
}

// IDA_EA: 0x8423528
// IDA_NAME: _ZN16DB_MouseRegister15_modifyPasswordEjPKcPcib
// DEMANGLED: DB_MouseRegister::_modifyPassword(unsigned int,char const*,char *,int,bool)
int DB_MouseRegister::_modifyPassword(unsigned int account_id,
                                      const char* encrypted_password,
                                      char* ip,
                                      int user_idx,
                                      bool advice_change_password) {
    if (_updateHistory(account_id, 9, ip) != true) {
        return 21;
    }
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 21;
    }
    db_handle->set_query(
        "upDate member_mousepass set mousepass='%s',occ_time=now() where m_id=%s and enable_flag = '1'",
        encrypted_password,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        return 21;
    }

    MySQL* account_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!account_db) {
        return 21;
    }
    account_db->set_query(
        "upDate member_security_grade set goblin_pass_mod=now() where m_id=%s",
        NumberToString(account_id, 0));
    if (account_db->exec(true) != true || !account_db->getAffectedRowCount()) {
        account_db->set_query(
            "inSert into member_security_grade(m_id, goblin_pass_mod) values(%s, now())",
            NumberToString(account_id, 0));
        if (account_db->exec(true) != true) {
            return 0;
        }
    }
    if (advice_change_password) {
        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 24013);
        CStreamGuard guard(out_stream, true);
        *guard << kGoblinPadAdviceChangePwdSignal;
        *guard << user_idx;
        SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD* out_payload =
            guard->GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>();
        if (!out_payload) {
            return 0;
        }
        out_payload->advice_change_password =
            advice_change_password ? 1u : 0u;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return 0;
}

// IDA_EA: 0x8423794
// IDA_NAME: _ZN16DB_MouseRegister17_unregistPasswordEjPc
// DEMANGLED: DB_MouseRegister::_unregistPassword(unsigned int,char *)
int DB_MouseRegister::_unregistPassword(unsigned int account_id, char* ip) {
    if (_updateHistory(account_id, 10, ip) != true) {
        return 21;
    }
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return 21;
    }
    db_handle->set_query("deLete from member_mousepass where m_id=%s",
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        return 21;
    }

    MySQL* account_db =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!account_db) {
        return 0;
    }
    account_db->set_query(
        "upDate member_security_grade set goblin_pass_mod='0000-00-00 00:00:00' where m_id=%s",
        NumberToString(account_id, 0));
    bool need_insert = true;
    if (account_db->exec(true) == true && account_db->getAffectedRowCount()) {
        need_insert = false;
    }
    if (!need_insert) {
        return 0;
    }
    account_db->set_query(
        "inSert into member_security_grade(m_id, goblin_pass_mod) values(%s, '0000-00-00 00:00:00')",
        NumberToString(account_id, 0));
    account_db->exec(true);
    return 0;
}

// IDA_EA: 0x8423914
// IDA_NAME: _ZN16DB_MouseRegister14_updateHistoryEjiPc
// DEMANGLED: DB_MouseRegister::_updateHistory(unsigned int,int,char *)
bool DB_MouseRegister::_updateHistory(unsigned int account_id,
                                      int modify_type,
                                      char* ip) {
    MySQL* db_handle = _getDBHandleForDevideAccountDB();
    if (!db_handle) {
        return false;
    }
    db_handle->set_query("seLect mousepass from member_mousepass where m_id=%s",
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows() ||
        db_handle->fetch() != true) {
        return false;
    }

    char previous_password[36];
    memset(previous_password, 0, sizeof(previous_password));
    if (db_handle->get_str(0, previous_password, 36) != true) {
        return false;
    }

    char month[10];
    memset(month, 0, sizeof(month));
    time_t timer = GlobalData::system_time()
                       ? GlobalData::system_time()->getCurSec()
                       : time(0);
    struct tm tp;
    localtime_r(&timer, &tp);
    strftime(month, sizeof(month), "%Y%m", &tp);

    db_handle->set_query(
        "inSert into member_mousepass_history_%s (occ_time, m_id, pre_mousepass, modify_type, ip_info, port_info) values (now(),%s,'%s',%d,'%s','')",
        month,
        NumberToString(account_id, 0),
        previous_password,
        modify_type,
        ip ? ip : "");
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    return false;
}

// IDA_EA: 0x8423ABE
// IDA_NAME: _ZN16DB_MouseRegister11_sendResultEjiiiP18SIG_MOUSE_REGISTER
// DEMANGLED: DB_MouseRegister::_sendResult(unsigned int,int,int,int,SIG_MOUSE_REGISTER *)
void DB_MouseRegister::_sendResult(unsigned int account_id,
                                   int result,
                                   int request_kind,
                                   int user_idx,
                                   SIG_MOUSE_REGISTER* payload_data) {
    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 24142);
    CStreamGuard guard(out_stream, true);
    *guard << kMouseRegisterInterSignal;
    *guard << user_idx;
    SIG_MOUSE_REGISTER* out_payload =
        guard->GetInBuffer<SIG_MOUSE_REGISTER>();
    if (!out_payload) {
        return;
    }
    memcpy(out_payload, payload_data, sizeof(*out_payload));
    out_payload->account_id = account_id;
    out_payload->request_kind = request_kind;
    out_payload->result = result;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x844E4B6
// IDA_NAME: _ZN16DB_MouseRegisterC2Ev
// DEMANGLED: DB_MouseRegister::DB_MouseRegister(void)
DB_MouseRegister::DB_MouseRegister() {}

// IDA_EA: 0x845EFF2
// IDA_NAME: _ZN16DB_MouseRegisterD2Ev
// DEMANGLED: DB_MouseRegister::~DB_MouseRegister()
// IDA_EA: 0x845F022
// IDA_NAME: _ZN16DB_MouseRegisterD0Ev
// DEMANGLED: DB_MouseRegister::~DB_MouseRegister() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_MouseRegister::~DB_MouseRegister() {}
