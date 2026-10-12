#include "old_data/Login/simple_sso.h"

#include "common/manager/global_data.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/system_time.h"

#include <cstring>

// IDA_DATA: GlobalData::s_psimpleSSO
// IDA_NAME: _ZN10GlobalData12s_psimpleSSOE
// DEMANGLED: GlobalData::s_psimpleSSO
WongWork::CSimpleSSO* GlobalData::s_psimpleSSO = 0;

namespace WongWork {

// IDA_DATA: 0x93AC220
// IDA_NAME: _ZN8WongWork10CSimpleSSO12strKeyTable_E
const char* const CSimpleSSO::strKeyTable_[31][24] = {
#include "old_data/Login/simple_sso_key_table.inc"
};

// IDA_EA: 0x818FB28
// IDA_NAME: _ZN8WongWork10CSimpleSSO14stOutputData_tC2Ev
// DEMANGLED: WongWork::CSimpleSSO::stOutputData_t::stOutputData_t(void)
CSimpleSSO::stOutputData_t::stOutputData_t()
    : last_play_time(0),
      account_fail(0),
      total_account_fail(0),
      report_count(0),
      reliable_flag(0),
      trade_gold_daily(0),
      security_flag(0),
      dungeon_gain_gold(0),
      school_id(0),
      rating(0.0f),
      tutorial_skipable(0),
      event_charac_flag(0),
      login_time(0),
      expire_time(0) {
    login_ip[0] = 0;
}

// IDA_EA: 0x82A7260
// IDA_NAME: _ZN8WongWork10CSimpleSSOC2Ev
// DEMANGLED: WongWork::CSimpleSSO::CSimpleSSO(void)
CSimpleSSO::CSimpleSSO() : rijndael_(), mysql_(0) {}

// IDA_EA: 0x82A7352
// IDA_NAME: _ZN8WongWork10CSimpleSSOD2Ev
// DEMANGLED: WongWork::CSimpleSSO::~CSimpleSSO()
CSimpleSSO::~CSimpleSSO() {}

// IDA_EA: 0x86027FC
// IDA_NAME: _ZN8WongWork10CSimpleSSO4initEP5MySQL
// DEMANGLED: WongWork::CSimpleSSO::init(MySQL *)
bool CSimpleSSO::init(MySQL* mysql) {
    mysql_ = mysql;
    for (int day = 0; day <= 30; ++day) {
        for (int hour = 0; hour <= 23; ++hour) {
            rijndael_[day][hour].Initialize(strKeyTable_[day][hour],
                                            32,
                                            CRijndael::sm_chain0,
                                            16,
                                            16,
                                            0,
                                            0);
        }
    }
    return true;
}

// IDA_EA: 0x86028B4
// IDA_NAME: _ZN8WongWork10CSimpleSSO5resetEv
// DEMANGLED: WongWork::CSimpleSSO::reset(void)
CSimpleSSO* CSimpleSSO::reset() {
    mysql_ = 0;
    return this;
}

// IDA_EA: 0x86028C6
// IDA_NAME: _ZN8WongWork10CSimpleSSO18checkValidPasswordEjPKcS2_RNS0_14stOutputData_tE
// DEMANGLED: WongWork::CSimpleSSO::checkValidPassword(unsigned int,char const*,char const*,WongWork::CSimpleSSO::stOutputData_t &)
int CSimpleSSO::checkValidPassword(unsigned int account_id,
                                   const char*,
                                   const char*,
                                   stOutputData_t& output) {
    long login_time = 0;
    long expire_time = 0;
    return _getGameStartTime(account_id, login_time, expire_time, output);
}

// IDA_EA: 0x8602906
// IDA_NAME: _ZN8WongWork10CSimpleSSO19updateSSOExpireTimeEjRKNS0_23stUpdateSSOExpireTime_tE
// DEMANGLED: WongWork::CSimpleSSO::updateSSOExpireTime(unsigned int,WongWork::CSimpleSSO::stUpdateSSOExpireTime_t const&)
bool CSimpleSSO::updateSSOExpireTime(
    unsigned int account_id,
    const stUpdateSSOExpireTime_t& update) {
    char table_name[44];
    std::memset(table_name, 0, 20);
    std::memcpy(table_name, "member_login", 13);
    const char* account_text = NumberToString(account_id, 0);
    const int last_play_time = GlobalData::system_time()->getCurSec();
    const int expire_time = GlobalData::system_time()->getCurSec();
    mysql_->set_query(
        "upDate %s set expire_time=%d,last_play_time=%d,report_cnt=%d,trade_gold_daily=%d,dungeon_gain_gold=%d,rating=%f,tutorial_skipable=%d where m_id=%s",
        table_name,
        expire_time + 180,
        last_play_time,
        update.report_count,
        update.trade_gold_daily,
        update.dungeon_gain_gold,
        static_cast<double>(update.rating),
        static_cast<int>(update.tutorial_skipable),
        account_text);
    if (!mysql_->exec(true)) {
        return false;
    }
    return mysql_->getAffectedRowCount() != 0;
}

// IDA_EA: 0x8602A4A
// IDA_NAME: _ZN8WongWork10CSimpleSSO19updateSSOExpireTimeEjj
// DEMANGLED: WongWork::CSimpleSSO::updateSSOExpireTime(unsigned int,unsigned int)
bool CSimpleSSO::updateSSOExpireTime(unsigned int account_id,
                                     unsigned int login_time) {
    char table_name[28];
    std::memset(table_name, 0, 20);
    std::memcpy(table_name, "member_login", 13);
    const char* account_text = NumberToString(account_id, 0);
    const int last_play_time = GlobalData::system_time()->getCurSec();
    const int expire_time = GlobalData::system_time()->getCurSec();
    mysql_->set_query(
        "upDate %s set login_time=%d, expire_time=%d, last_play_time=%d where m_id=%s",
        table_name,
        login_time,
        expire_time + 180,
        last_play_time,
        account_text);
    return mysql_->exec(true);
}

// IDA_EA: 0x8602B24
// IDA_NAME: _ZN8WongWork10CSimpleSSO15expireLoginTimeEj
// DEMANGLED: WongWork::CSimpleSSO::expireLoginTime(unsigned int)
void CSimpleSSO::expireLoginTime(unsigned int account_id) {
    char table_name[28];
    std::memset(table_name, 0, 20);
    std::memcpy(table_name, "member_login", 13);
    if (account_id != 0) {
        const char* account_text = NumberToString(account_id, 0);
        const int current_time =
            GlobalData::system_time()->getCurSec();
        mysql_->set_query("upDate %s set expire_time=%d where m_id=%s",
                          table_name,
                          current_time,
                          account_text);
        mysql_->exec(true);
    }
}

// IDA_EA: 0x8602DA6
// IDA_NAME: _ZN8WongWork10CSimpleSSO17_getGameStartTimeEjRlS1_RNS0_14stOutputData_tE
// DEMANGLED: WongWork::CSimpleSSO::_getGameStartTime(unsigned int,long &,long &,WongWork::CSimpleSSO::stOutputData_t &)
int CSimpleSSO::_getGameStartTime(unsigned int account_id,
                                  long& login_time,
                                  long& expire_time,
                                  stOutputData_t& output) {
    char table_name[20];
    std::memset(table_name, 0, sizeof(table_name));
    std::memcpy(table_name, "member_login", 13);
    const char* account_text = NumberToString(account_id, 0);
    mysql_->set_query(
        "seLect login_time,expire_time,report_cnt,last_play_time,account_fail,total_account_fail,reliable_flag,trade_gold_daily,login_ip,security_flag,dungeon_gain_gold,school_id,rating,tutorial_skipable,event_charac_flag from %s where m_id=%s",
        table_name,
        account_text);
    if (!mysql_->exec(true) || !mysql_->fetch()) {
        return 0;
    }
    if (mysql_->get_n_rows() == 0) {
        return 0;
    }

    unsigned int db_login_time = 0;
    unsigned int db_expire_time = 0;
    int column = 0;
    if (!mysql_->get_uint(column++, db_login_time) ||
        !mysql_->get_uint(column++, db_expire_time) ||
        !mysql_->get_int(column++, output.report_count) ||
        !mysql_->get_int(column++, output.last_play_time) ||
        !mysql_->get_int(column++, output.account_fail) ||
        !mysql_->get_int(column++, output.total_account_fail) ||
        !mysql_->get_byte(column++, output.reliable_flag) ||
        !mysql_->get_uint(column++, output.trade_gold_daily) ||
        !mysql_->get_str(column++, output.login_ip, 20) ||
        !mysql_->get_byte(column++, output.security_flag) ||
        !mysql_->get_uint(column++, output.dungeon_gain_gold) ||
        !mysql_->get_int(column++, output.school_id) ||
        !mysql_->get_float(column++, output.rating) ||
        !mysql_->get_ubyte(column++, output.tutorial_skipable) ||
        !mysql_->get_ubyte(column++, output.event_charac_flag)) {
        return 0;
    }
    login_time = static_cast<long>(db_login_time);
    expire_time = static_cast<long>(db_expire_time);
    output.login_time = db_login_time;
    output.expire_time = db_expire_time;
    return 1;
}

// IDA_EA: 0x86031F6
// IDA_NAME: _ZN8WongWork10CSimpleSSO16getGameStartTimeEjRlS1_RNS0_14stOutputData_tE
// DEMANGLED: WongWork::CSimpleSSO::getGameStartTime(unsigned int,long &,long &,WongWork::CSimpleSSO::stOutputData_t &)
int CSimpleSSO::getGameStartTime(unsigned int account_id,
                                 long& login_time,
                                 long& expire_time,
                                 stOutputData_t& output) {
    return _getGameStartTime(account_id, login_time, expire_time, output);
}

// IDA_EA: 0x8603226
// IDA_NAME: _ZN8WongWork10CSimpleSSO18updateHumanCertifyEji
// DEMANGLED: WongWork::CSimpleSSO::updateHumanCertify(unsigned int,int)
bool CSimpleSSO::updateHumanCertify(unsigned int account_id,
                                    int account_fail) {
    char table_name[20];
    std::memset(table_name, 0, sizeof(table_name));
    std::memcpy(table_name, "member_login", 13);
    const int increment_total = account_fail != 0;
    const char* account_text = NumberToString(account_id, 0);
    mysql_->set_query(
        "upDate %s set last_play_time=unix_timestamp(now()),account_fail=%d,total_account_fail=total_account_fail+%d where m_id=%s",
        table_name,
        account_fail,
        increment_total,
        account_text);
    if (!mysql_->exec(true)) {
        return false;
    }
    return mysql_->getAffectedRowCount() != 0;
}

}  // namespace WongWork
