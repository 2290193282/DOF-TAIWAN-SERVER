#include "old_data/db/core/mysql.h"

#include "infra/concurrent/timer/query_counter.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

struct MYSQL;
struct MYSQL_RES;
typedef char** MYSQL_ROW;

extern "C" MYSQL* mysql_init(MYSQL*);
extern "C" int mysql_options(MYSQL*, int, const void*);
extern "C" MYSQL* mysql_real_connect(MYSQL*, const char*, const char*,
                                      const char*, const char*, unsigned int,
                                      const char*, unsigned long);
extern "C" void mysql_close(MYSQL*);
extern "C" unsigned int mysql_errno(MYSQL*);
extern "C" int mysql_real_query(MYSQL*, const char*, unsigned long);
extern "C" MYSQL_RES* mysql_store_result(MYSQL*);
extern "C" unsigned long long mysql_num_rows(MYSQL_RES*);
extern "C" unsigned int mysql_num_fields(MYSQL_RES*);
extern "C" MYSQL_ROW mysql_fetch_row(MYSQL_RES*);
extern "C" unsigned long* mysql_fetch_lengths(MYSQL_RES*);
extern "C" void mysql_free_result(MYSQL_RES*);
extern "C" int mysql_ping(MYSQL*);
extern "C" unsigned long mysql_real_escape_string(MYSQL*, char*, const char*,
                                                  unsigned long);
extern "C" const char* mysql_get_server_info(MYSQL*);
extern "C" const char* mysql_get_client_info();
extern "C" unsigned long long mysql_affected_rows(MYSQL*);


class CDnFTimer {
public:
    virtual void SetLastTime();
    virtual long double GetTimeInterval();
};

class CDnFTimerFactory {
public:
    static CDnFTimer* CreateDnFTimer();
    static void DestroyDnFTimer(CDnFTimer* timer);
};

class LogManager {
public:
    static void logFormat(int log_index, const char* file,
                          const char* function, int line,
                          const char* format, ...);
};

class cMyTrace {
public:
    cMyTrace(const char* function, int line, int level);
    cMyTrace(const char* function, int line, int level, bool prefix,
             bool newline);
    int operator()(const char* format, ...);

private:
    char storage_[24];
};

class cMyTraceNoop {
public:
    cMyTraceNoop();
    int operator()(const char* format = "", ...);

private:
    char storage_[1];
};

typedef char MySQL_Size_Must_Be_0x4208C[(sizeof(MySQL) == 0x4208C) ? 1 : -1];


// IDA_EA: 0x80E22AE
// IDA_NAME: _ZN5MySQL7get_intEiRj
// DEMANGLED: MySQL::get_int(int,unsigned int &)
bool MySQL::get_int(int column, unsigned int& out) {
    return get_uint(column, out);
}

// IDA_EA: 0x80E22D0
// IDA_NAME: _ZN5MySQL12is_valid_colEi
// DEMANGLED: MySQL::is_valid_col(int)
bool MySQL::is_valid_col(int column) {
    return column >= 0 && fields_ > static_cast<unsigned int>(column);
}

// IDA_EA: 0x80E22F2
// IDA_NAME: _ZN5MySQL8get_uintEiRj
// DEMANGLED: MySQL::get_uint(int,unsigned int &)
bool MySQL::get_uint(int column, unsigned int& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<unsigned int>(std::strtoul(row_[column], 0, 10));
    return true;
}

// IDA_EA: 0x80E236C
// IDA_NAME: _ZN5MySQL10get_n_rowsEv
// DEMANGLED: MySQL::get_n_rows(void)
int MySQL::get_n_rows() {
    return static_cast<int>(rows_);
}

// IDA_EA: 0x80ECDEA
// IDA_NAME: _ZN5MySQL7get_strEiPci
// DEMANGLED: MySQL::get_str(int,char *,int)
bool MySQL::get_str(int column, char* dest, int n) {
    if (!row_ || !is_valid_col(column) || n <= 0) {
        return false;
    }
    std::strncpy(dest, row_[column], static_cast<unsigned int>(n));
    dest[n - 1] = 0;
    return true;
}

// IDA_EA: 0x80FDDEC
// IDA_NAME: _ZN5MySQL9get_shortEiRs
// DEMANGLED: MySQL::get_short(int,short &)
bool MySQL::get_short(int column, short& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<short>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x80FDE54
// IDA_NAME: _ZN5MySQL8get_byteEiRc
// DEMANGLED: MySQL::get_byte(int,char &)
bool MySQL::get_byte(int column, char& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<char>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x80FDEBA
// IDA_NAME: _ZN5MySQL9get_ubyteEiRh
// DEMANGLED: MySQL::get_ubyte(int,unsigned char &)
bool MySQL::get_ubyte(int column, unsigned char& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<unsigned char>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x811692C
// IDA_NAME: _ZN5MySQL7get_intEiRi
// DEMANGLED: MySQL::get_int(int,int &)
bool MySQL::get_int(int column, int& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = std::atoi(row_[column]);
    return true;
}

// IDA_EA: 0x8116990
// IDA_NAME: _ZN5MySQL10get_ushortEiRt
// DEMANGLED: MySQL::get_ushort(int,unsigned short &)
bool MySQL::get_ushort(int column, unsigned short& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<unsigned short>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x811B97A
// IDA_NAME: _ZN5MySQL10getDBErrorEv
// DEMANGLED: MySQL::getDBError(void)
int MySQL::getDBError() {
    return db_error_;
}

// IDA_EA: 0x812531A
// IDA_NAME: _ZN5MySQL10get_binaryEiPvi
// DEMANGLED: MySQL::get_binary(int,void *,int)
bool MySQL::get_binary(int column, void* dest, int n) {
    unsigned long copy_size;

    if (!row_ || !is_valid_col(column) || n <= 0) {
        return false;
    }
    if (lengths_[column] >= static_cast<unsigned long>(n)) {
        copy_size = static_cast<unsigned long>(n);
    } else {
        copy_size = lengths_[column];
        std::memset(dest, 0, static_cast<unsigned int>(n));
    }
    std::memcpy(dest, row_[column], copy_size);
    return true;
}

// IDA_EA: 0x81253DE
// IDA_NAME: _ZN5MySQL17get_binary_lengthEi
// DEMANGLED: MySQL::get_binary_length(int)
int MySQL::get_binary_length(int column) {
    return static_cast<int>(lengths_[column]);
}

// IDA_EA: 0x814201C
// IDA_NAME: _ZN5MySQL9get_shortEiRi
// DEMANGLED: MySQL::get_short(int,int &)
bool MySQL::get_short(int column, int& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<short>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x815E6A6
// IDA_NAME: _ZN5MySQL8get_charEiRc
// DEMANGLED: MySQL::get_char(int,char &)
bool MySQL::get_char(int column, char& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = *row_[column];
    return true;
}

// IDA_EA: 0x81754C8
// IDA_NAME: _ZN5MySQL13get_ulonglongEiRy
// DEMANGLED: MySQL::get_ulonglong(int,unsigned long long &)
bool MySQL::get_ulonglong(int column, unsigned long long& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = std::strtoull(row_[column], 0, 10);
    return true;
}

// IDA_EA: 0x82A4202
// IDA_NAME: _ZN5MySQL15GetQueryCounterEv
// DEMANGLED: MySQL::GetQueryCounter(void)
CQueryCounter* MySQL::GetQueryCounter() {
    return query_counter_;
}

// IDA_EA: 0x83F3AC8
// IDA_NAME: _ZN5MySQLC2Ev
// DEMANGLED: MySQL::MySQL(void)
MySQL::MySQL() {
    mysql_handle_ = 0;
    query_counter_ = 0;
    query_counter_enabled_ = 0;
}

// IDA_EA: 0x83F3AEA
// IDA_NAME: _ZN5MySQLD2Ev
// DEMANGLED: MySQL::~MySQL()
MySQL::~MySQL() {
    close();
}

// IDA_EA: 0x83F3AFE
// IDA_NAME: _ZN5MySQL19set_compress_optionEv
// DEMANGLED: MySQL::set_compress_option(void)
bool MySQL::set_compress_option() {
    if (!mysql_options(static_cast<MYSQL*>(mysql_handle_), 1, 0)) {
        return true;
    }
    cMyTrace trace("bool MySQL::set_compress_option()", 64, 5);
    trace("[%s][%d]", "bool MySQL::set_compress_option()", 64);
    return false;
}

// IDA_EA: 0x83F3B80
// IDA_NAME: _ZN5MySQL27set_read_default_grp_optionEv
// DEMANGLED: MySQL::set_read_default_grp_option(void)
bool MySQL::set_read_default_grp_option() {
    if (!mysql_options(static_cast<MYSQL*>(mysql_handle_), 5, "UseSQL")) {
        return true;
    }
    cMyTrace trace("bool MySQL::set_read_default_grp_option()", 79, 5);
    trace("[%s][%d]", "bool MySQL::set_read_default_grp_option()", 79);
    return false;
}

// IDA_EA: 0x83F3C02
// IDA_NAME: _ZN5MySQL23set_charset_name_optionEv
// DEMANGLED: MySQL::set_charset_name_option(void)
bool MySQL::set_charset_name_option() {
    return true;
}

// IDA_EA: 0x83F3C0C
// IDA_NAME: _ZN5MySQL20set_reconnect_optionEv
// DEMANGLED: MySQL::set_reconnect_option(void)
bool MySQL::set_reconnect_option() {
    return true;
}

// IDA_EA: 0x83F3C16
// IDA_NAME: _ZN5MySQL14init_db_handleEv
// DEMANGLED: MySQL::init_db_handle(void)
bool MySQL::init_db_handle() {
    if (mysql_handle_) {
        cMyTrace trace("bool MySQL::init_db_handle()", 129, 5);
        trace("[%s][%d]", "bool MySQL::init_db_handle()", 129);
        return false;
    }

    mysql_handle_ = mysql_init(0);
    if (mysql_handle_) {
        return true;
    }

    cMyTrace trace("bool MySQL::init_db_handle()", 140, 5);
    trace("[%s][%d]", "bool MySQL::init_db_handle()", 140);
    return false;
}

// IDA_EA: 0x83F3CE4
// IDA_NAME: _ZN5MySQL4initEv
// DEMANGLED: MySQL::init(void)
bool MySQL::init() {
    if (init_db_handle() != true) {
        return false;
    }
    if (set_compress_option() != true) {
        return false;
    }
    if (set_read_default_grp_option() != true) {
        return false;
    }
    if (set_charset_name_option() != true) {
        return false;
    }
    if (set_reconnect_option() != true) {
        return false;
    }

    std::memset(query_, 0, 0x6001u);
    query_length_ = 0;
    result_ = 0;
    lengths_ = 0;
    rows_ = 0;
    fields_ = 0;
    std::memset(db_host_, 0, 0x10u);
    std::memset(db_user_, 0, 0x14u);
    std::memset(db_name_, 0, 0x1Eu);
    std::memset(db_passwd_, 0, 0x14u);
    return true;
}

// IDA_EA: 0x83F3E74
// IDA_NAME: _ZN5MySQL5closeEv
// DEMANGLED: MySQL::close(void)
MySQL* MySQL::close() {
    MySQL* result = static_cast<MySQL*>(mysql_handle_);
    if (result) {
        mysql_close(static_cast<MYSQL*>(mysql_handle_));
        result = this;
        mysql_handle_ = 0;
    }
    return result;
}

// IDA_EA: 0x83F3E9E
// IDA_NAME: _ZN5MySQL4openEPcS0_S0_S0_
// DEMANGLED: MySQL::open(char *,char *,char *,char *)
bool MySQL::open(char* host, char* database, char* user, char* password) {
    std::strcpy(db_name_, database);
    std::strcpy(db_host_, host);
    std::strcpy(db_user_, user);
    std::strcpy(db_passwd_, password);
    if (mysql_real_connect(static_cast<MYSQL*>(mysql_handle_), host, user, password,
                           database, 3306, 0, 1024)) {
        return true;
    }
    mysql_errno(static_cast<MYSQL*>(mysql_handle_));
    cMyTrace trace_connect("bool MySQL::open(char*, char*, char*, char*)",
                           266, 9, 1, 1);
    trace_connect(
        "\n\t- mysql_real_connect(ip:%s, port:%d, name:%s, id:%s error='%d') fail",
        host, 3306, database, user, mysql_errno(static_cast<MYSQL*>(mysql_handle_)));
    cMyTrace trace("bool MySQL::open(char*, char*, char*, char*)", 270, 5);
    trace("[%s][%d]", "bool MySQL::open(char*, char*, char*, char*)", 270);
    return false;
}

// IDA_EA: 0x83F4024
// IDA_NAME: _ZN5MySQL4openEPcjS0_S0_S0_
// DEMANGLED: MySQL::open(char *,unsigned int,char *,char *,char *)
bool MySQL::open(char* host, unsigned int port, char* database, char* user,
                 char* password) {
    if (!port) {
        port = 3306;
    }
    std::strcpy(db_name_, database);
    std::strcpy(db_host_, host);
    db_port_ = port;
    std::strcpy(db_user_, user);
    std::strcpy(db_passwd_, password);
    if (mysql_real_connect(static_cast<MYSQL*>(mysql_handle_), host, user, password,
                           database, port, 0, 0)) {
        return true;
    }
    mysql_errno(static_cast<MYSQL*>(mysql_handle_));
    cMyTrace trace_connect(
        "bool MySQL::open(char*, unsigned int, char*, char*, char*)",
        312, 9, 1, 1);
    trace_connect(
        "\n\t- mysql_real_connect(ip:%s, port:%d, name:%s, id:%s error='%d') fail",
        host, port, database, user, mysql_errno(static_cast<MYSQL*>(mysql_handle_)));
    cMyTrace trace(
        "bool MySQL::open(char*, unsigned int, char*, char*, char*)",
        316, 5);
    trace("[%s][%d]",
          "bool MySQL::open(char*, unsigned int, char*, char*, char*)",
          316);
    return false;
}

// IDA_EA: 0x83F41C0
// IDA_NAME: _ZN5MySQL9set_queryEPKcz
// DEMANGLED: MySQL::set_query(char const*,...)
bool MySQL::set_query(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if (query_counter_enabled_) {
        if (!query_counter_) {
            va_end(args);
            return false;
        }
        std::string query_template(format);
        query_counter_->IncreaseQueryCount(query_template);
    }
    std::vsprintf(query_, format, args);
    va_end(args);

    const int length = static_cast<int>(std::strlen(query_));
    if (length <= 24575) {
        query_length_ = length;
        return true;
    }

    cMyTrace trace("bool MySQL::set_query(const char*, ...)", 361, 5);
    trace("[%s][%d]", "bool MySQL::set_query(const char*, ...)", 361);
    return false;
}

// IDA_EA: 0x83F4310
// IDA_NAME: _ZN5MySQL19getAffectedRowCountEv
// DEMANGLED: MySQL::getAffectedRowCount(void)
// The old i386 callers consume the mysql_affected_rows result in EDX:EAX.
unsigned long long MySQL::getAffectedRowCount() {
    return mysql_affected_rows(static_cast<MYSQL*>(mysql_handle_));
}

// IDA_EA: 0x83F4326
// IDA_NAME: _ZN5MySQL4execEb
// DEMANGLED: MySQL::exec(bool)
bool MySQL::exec(bool log_error) {
    int query_status = 0;
    CDnFTimer* timer = CDnFTimerFactory::CreateDnFTimer();
    if (timer) {
        timer->SetLastTime();
    }

    for (int retry = 0; retry <= 4; ++retry) {
        query_status = exec_query();
        if (query_status == 1) {
            break;
        }
        if (!query_status) {
            break;
        }
    }

    if (query_counter_enabled_) {
        const long double elapsed = timer ? timer->GetTimeInterval() : 0.0;
        if (query_counter_) {
            query_counter_->SetResponseTime(static_cast<double>(elapsed));
        }
    }
    if (timer) {
        CDnFTimerFactory::DestroyDnFTimer(timer);
    }

    if (query_status) {
        if (log_error) {
            LogManager::logFormat(
                1,
                "DBMgr.cpp",
                "bool MySQL::exec(bool)",
                0x244,
                "Database query error. The last query('%s') has been lost.(%d)",
                query_,
                db_error_);
        }
        return false;
    }

    result_ = mysql_store_result(static_cast<MYSQL*>(mysql_handle_));
    if (result_) {
        rows_ = static_cast<unsigned int>(mysql_num_rows(static_cast<MYSQL_RES*>(result_)));
        fields_ = mysql_num_fields(static_cast<MYSQL_RES*>(result_));
    } else {
        rows_ = 0;
        fields_ = 0;
    }
    return true;
}

// IDA_EA: 0x83F44BC
// IDA_NAME: _ZN5MySQL5fetchEv
// DEMANGLED: MySQL::fetch(void)
bool MySQL::fetch() {
    if (!result_) {
        return false;
    }
    row_ = mysql_fetch_row(static_cast<MYSQL_RES*>(result_));
    if (!row_) {
        return false;
    }
    lengths_ = mysql_fetch_lengths(static_cast<MYSQL_RES*>(result_));
    return true;
}

// IDA_EA: 0x83F4514
// IDA_NAME: _ZN5MySQL4pingEv
// DEMANGLED: MySQL::ping(void)
int MySQL::ping() {
    return mysql_ping(static_cast<MYSQL*>(mysql_handle_));
}

// IDA_EA: 0x83F452A
// IDA_NAME: _ZN5MySQL11blob_to_strEiPvi
// DEMANGLED: MySQL::blob_to_str(int,void *,int)
char* MySQL::blob_to_str(int index, void* data, int n) {
    char* out;

    if (index > 9 || !data || n > 24575) {
        cMyTrace trace("char* MySQL::blob_to_str(int, void*, int)", 615, 5);
        trace("[%s][%d]", "char* MySQL::blob_to_str(int, void*, int)", 615);
    }
    blob_buffers_[index][0] = 0;
    if (n > 0) {
        out = blob_buffers_[index];
        out += mysql_real_escape_string(static_cast<MYSQL*>(mysql_handle_), out,
                                        static_cast<const char*>(data),
                                        static_cast<unsigned long>(n));
        *out++ = 0;
    }
    return blob_buffers_[index];
}

// IDA_EA: 0x83F460C
// IDA_NAME: _ZN5MySQL16getServerVersionEv
// DEMANGLED: MySQL::getServerVersion(void)
const char* MySQL::getServerVersion() {
    return mysql_get_server_info(static_cast<MYSQL*>(mysql_handle_));
}

// IDA_EA: 0x83F4622
// IDA_NAME: _ZN5MySQL16getClientVersionEv
// DEMANGLED: MySQL::getClientVersion(void)
const char* MySQL::getClientVersion() {
    return mysql_get_client_info();
}

// IDA_EA: 0x83F4630
// IDA_NAME: _ZN5MySQL6selectEv
// DEMANGLED: MySQL::select(void)
int MySQL::select() {
    if (set_query() == true) {
        return exec(true) != true;
    }
    return 3;
}

// IDA_EA: 0x83F4678
// IDA_NAME: _ZN5MySQL6selectEPKcz
// DEMANGLED: MySQL::select(char const*,...)
int MySQL::select(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vsprintf(query_, format, args);
    va_end(args);
    return select();
}

// IDA_EA: 0x83F46AE
// IDA_NAME: _ZN5MySQL6insertEPKcz
// DEMANGLED: MySQL::insert(char const*,...)
int MySQL::insert(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vsprintf(query_, format, args);
    va_end(args);
    if (set_query() != true) {
        return 3;
    }
    if (exec(true) == true) {
        return 0;
    }
    return 5;
}

// IDA_EA: 0x83F4718
// IDA_NAME: _ZN5MySQL6updateEPKcz
// DEMANGLED: MySQL::update(char const*,...)
int MySQL::update(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vsprintf(query_, format, args);
    va_end(args);
    if (set_query() != true) {
        return 3;
    }
    if (exec(true) == true) {
        return 0;
    }
    return 5;
}

// IDA_EA: 0x83F4782
// IDA_NAME: _ZN5MySQL9set_queryEv
// DEMANGLED: MySQL::set_query(void)
bool MySQL::set_query() {
    if (query_counter_enabled_) {
        if (!query_counter_) {
            return false;
        }
        std::string query_text(query_);
        query_counter_->IncreaseQueryCount(query_text);
    }

    const int length = static_cast<int>(std::strlen(query_));
    if (length <= 24575) {
        query_length_ = length;
        return true;
    }

    LogManager::logFormat(1,
                          "DBMgr.cpp",
                          "bool MySQL::set_query()",
                          0x2D1,
                          "[%s][%d]",
                          "bool MySQL::set_query()",
                          721);
    return false;
}

// IDA_EA: 0x83F48AA
// IDA_NAME: _ZN5MySQL13escape_stringEPcPKc
// DEMANGLED: MySQL::escape_string(char *,char const*)
int MySQL::escape_string(char* dest, const char* src) {
    const unsigned long n = static_cast<unsigned long>(std::strlen(src));
    return static_cast<int>(
        mysql_real_escape_string(static_cast<MYSQL*>(mysql_handle_), dest, src, n));
}

// IDA_EA: 0x83F531C
// IDA_NAME: _ZN5MySQL15SetQueryCounterEP13CQueryCounter
// DEMANGLED: MySQL::SetQueryCounter(CQueryCounter *)
MySQL* MySQL::SetQueryCounter(CQueryCounter* counter) {
    query_counter_ = counter;
    if (query_counter_) {
        query_counter_enabled_ = 1;
        return this;
    }
    return 0;
}

// IDA_EA: 0x83F5348
// IDA_NAME: _ZN5MySQL10exec_queryEv
// DEMANGLED: MySQL::exec_query(void)
int MySQL::exec_query() {
    char reconnect_ping;
    unsigned int db_errno_value;

    clear_result_set();
    if (!mysql_real_query(static_cast<MYSQL*>(mysql_handle_), query_,
                          static_cast<unsigned long>(query_length_))) {
        return 0;
    }

    db_error_ = static_cast<int>(mysql_errno(static_cast<MYSQL*>(mysql_handle_)));
    if (db_error_ == 2005 || db_error_ == 2013 || db_error_ == 2006) {
        cMyTraceNoop noop;
        noop();
        reconnect_ping = static_cast<char>(mysql_ping(static_cast<MYSQL*>(mysql_handle_)));
        if (reconnect_ping && mysql_errno(static_cast<MYSQL*>(mysql_handle_)) == 2006) {
            if (mysql_real_connect(static_cast<MYSQL*>(mysql_handle_),
                                   db_host_,
                                   db_user_,
                                   db_passwd_,
                                   db_name_,
                                   db_port_,
                                   0,
                                   0)) {
                LogManager::logFormat(1,
                                      "DBMgr.cpp",
                                      "int MySQL::exec_query()",
                                      0x19E,
                                      "DB Reconnect By Server Gone Error");
            } else {
                db_errno_value =
                    mysql_errno(static_cast<MYSQL*>(mysql_handle_));
                LogManager::logFormat(1,
                                      "DBMgr.cpp",
                                      "int MySQL::exec_query()",
                                      0x19A,
                                      "DB reconnection fail. err_no(%d)",
                                      db_errno_value);
            }
            LogManager::logFormat(
                1,
                "DBMgr.cpp",
                "int MySQL::exec_query()",
                0x1A1,
                "DB reconnection fail. %d (dbname : %s, ip : %s, port : %d, id : %s)",
                reconnect_ping,
                db_name_,
                db_host_,
                db_port_,
                db_user_);
        }
        return 2;
    }

    if (db_error_ != 1062) {
        db_errno_value =
            mysql_errno(static_cast<MYSQL*>(mysql_handle_));
        LogManager::logFormat(1,
                              "DBMgr.cpp",
                              "int MySQL::exec_query()",
                              0x1AF,
                              "DB error occured (%d) Query('%s')",
                              db_errno_value,
                              query_);
        if (db_error_ == 2006) {
            cMyTrace trace("int MySQL::exec_query()", 434, 5);
            trace("DB error occured Check Connection First, Must Be Not Connected!");
        }
    }
    return 1;
}

// IDA_EA: 0x83F5630
// IDA_NAME: _ZN5MySQL16clear_result_setEv
// DEMANGLED: MySQL::clear_result_set(void)
MySQL* MySQL::clear_result_set() {
    if (result_) {
        mysql_free_result(static_cast<MYSQL_RES*>(result_));
    }
    result_ = 0;
    row_ = 0;
    return this;
}

// IDA_EA: 0x844D65E
// IDA_NAME: _ZN5MySQL14GetQueryStringEv
// DEMANGLED: MySQL::GetQueryString(void)
char* MySQL::GetQueryString() {
    return query_;
}

// IDA_EA: 0x844D66A
// IDA_NAME: _ZN5MySQL9get_ulongEiRm
// DEMANGLED: MySQL::get_ulong(int,unsigned long &)
bool MySQL::get_ulong(int column, unsigned long& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<unsigned long>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x844D6D0
// IDA_NAME: _ZN5MySQL9get_floatEiRf
// DEMANGLED: MySQL::get_float(int,float &)
bool MySQL::get_float(int column, float& out) {
    float value;

    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    value = static_cast<float>(std::atof(row_[column]));
    out = value;
    return true;
}

// IDA_EA: 0x844D73A
// IDA_NAME: _ZN5MySQL8get_byteEiRi
// DEMANGLED: MySQL::get_byte(int,int &)
bool MySQL::get_byte(int column, int& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = static_cast<char>(std::atoi(row_[column]));
    return true;
}

// IDA_EA: 0x85F41B2
// IDA_NAME: _ZN5MySQL13is_valid_dataEi
// DEMANGLED: MySQL::is_valid_data(int)
bool MySQL::is_valid_data(int column) {
    return row_ && is_valid_col(column) && row_[column] != 0;
}

// IDA_EA: 0x85F4214
// IDA_NAME: _ZN5MySQL12get_longlongEiRx
// DEMANGLED: MySQL::get_longlong(int,long long &)
bool MySQL::get_longlong(int column, long long& out) {
    if (!row_ || !is_valid_col(column)) {
        return false;
    }
    out = std::strtoll(row_[column], 0, 10);
    return true;
}
