#include "infra/log/format/clog.h"

#include "infra/log/format/trace.h"

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <memory>
#include <stddef.h>
#include <sys/stat.h>


CLog* CLog::this_ = 0;

static char kLogSuffix[] = ".log";
static char kErrorSuffix[] = ".error";
static char kDebugSuffix[] = ".debug";
static char kMoneySuffix[] = ".money";
static char kHistorySuffix[] = ".history";
static char kInitSuffix[] = ".init";
static char kCriticalSuffix[] = ".cri";
static char kSnapshotSuffix[] = ".snap";

static int setLogWriterWired() __attribute__((noinline));

// IDA_EA: 0x854E228
// IDA_NAME: _ZN8TimeLog_7setTimeEv
// DEMANGLED: TimeLog_::setTime(void)
TimeLog_* TimeLog_::setTime() {
    time_t timer = time(0);
    struct tm tp;
    localtime_r(&timer, &tp);
    year = static_cast<unsigned short>(tp.tm_year + 1900);
    month = static_cast<unsigned char>(tp.tm_mon + 1);
    day = static_cast<unsigned char>(tp.tm_mday);
    hour = static_cast<unsigned char>(tp.tm_hour);
    minute = static_cast<unsigned char>(tp.tm_min);
    second = static_cast<unsigned char>(tp.tm_sec);
    return this;
}

// IDA_EA: 0x854E558
// IDA_NAME: _ZN4CLogC2Ev
// DEMANGLED: CLog::CLog(void)
CLog::CLog() : rotate_hours_(0), history_rotate_hours_(0) {
#if __SIZEOF_POINTER__ == 4
    typedef char CLog_Size_Must_Be_0x358[
        (sizeof(CLog) == 0x358u) ? 1 : -1];
    typedef char CLog_Files_Offset_Must_Be_0x0F0[
        (__builtin_offsetof(CLog, files_) == 0x0F0u) ? 1 : -1];
    typedef char CLog_Path_Offset_Must_Be_0x110[
        (__builtin_offsetof(CLog, log_path_) == 0x110u) ? 1 : -1];
    typedef char CLog_Times_Offset_Must_Be_0x210[
        (__builtin_offsetof(CLog, times_) == 0x210u) ? 1 : -1];
    typedef char CLog_Rotate_Offset_Must_Be_0x250[
        (__builtin_offsetof(CLog, rotate_hours_) == 0x250u) ? 1 : -1];
    typedef char CLog_History_Rotate_Offset_Must_Be_0x254[
        (__builtin_offsetof(CLog, history_rotate_hours_) == 0x254u) ? 1 : -1];
    typedef char CLog_Prefix_Offset_Must_Be_0x258[
        (__builtin_offsetof(CLog, log_prefix_) == 0x258u) ? 1 : -1];
    (void)sizeof(CLog_Size_Must_Be_0x358);
    (void)sizeof(CLog_Files_Offset_Must_Be_0x0F0);
    (void)sizeof(CLog_Path_Offset_Must_Be_0x110);
    (void)sizeof(CLog_Times_Offset_Must_Be_0x210);
    (void)sizeof(CLog_Rotate_Offset_Must_Be_0x250);
    (void)sizeof(CLog_History_Rotate_Offset_Must_Be_0x254);
    (void)sizeof(CLog_Prefix_Offset_Must_Be_0x258);
#endif
    for (int index = 0; index < FILE_COUNT; ++index) {
        files_[index] = 0;
    }
    std::memset(log_path_, 0, sizeof(log_path_));
    std::memset(log_prefix_, 0, sizeof(log_prefix_));
    setLogWriterWired();
}

// IDA_EA: 0x854E69C
// IDA_NAME: _ZN4CLogD2Ev
// DEMANGLED: CLog::~CLog()
CLog::~CLog() {
    for (int index = 0; index < FILE_COUNT; ++index) {
        closeFile(static_cast<LogFileIndex>(index));
    }
}

// IDA_EA: 0x82A3CB3
// IDA_NAME: _ZN4CLog14createInstanceEv
// DEMANGLED: CLog::createInstance(void)
CLog* CLog::createInstance() {
    CLog* log = new CLog();
    this_ = log;
    return log;
}

// IDA_EA: 0x817DFEC
// IDA_NAME: _ZN4CLog8instanceEv
// DEMANGLED: CLog::instance(void)
CLog* CLog::instance() {
    return this_;
}

// IDA_EA: 0x82A3CFD
// IDA_NAME: _ZN4CLog15destroyInstanceEv
// DEMANGLED: CLog::destroyInstance(void)
void CLog::destroyInstance() {
    if (this_ != 0) {
        CLog* log = this_;
        delete log;
    }
    this_ = 0;
}

// IDA_EA: 0x854E88C
// IDA_NAME: _ZN4CLog8initiateEPcS0_i
// DEMANGLED: CLog::initiate(char *,char *,int)
CLog* CLog::initiate(char* path, char* src, int rotate_hours) {
    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0 && mkdir(path, 0775) < 0) {
        std::printf("Fail To Make Directory, PATH[%s]\n", path);
        return this;
    }

    rotate_hours_ = rotate_hours;
    history_rotate_hours_ = 1;
    if (src != 0) {
        std::memcpy(log_prefix_, src, 0x100u);
        log_prefix_[sizeof(log_prefix_) - 1] = '\0';
    } else {
        std::memcpy(log_prefix_, "Log", 4u);
    }

    for (int index = 0; index < FILE_COUNT; ++index) {
        times_[index].setTime();
    }

    if (path != 0) {
        std::strncpy(log_path_, path, sizeof(log_path_) - 1);
        log_path_[sizeof(log_path_) - 1] = '\0';
        openLogFile(times_[FILE_NOTICE], files_[FILE_NOTICE], kLogSuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_ERROR], files_[FILE_ERROR], kErrorSuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_DEBUG], files_[FILE_DEBUG], kDebugSuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_MONEY], files_[FILE_MONEY], kMoneySuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_HISTORY], files_[FILE_HISTORY], kHistorySuffix,
                    history_rotate_hours_);
        openLogFile(times_[FILE_INIT], files_[FILE_INIT], kInitSuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_CRITICAL], files_[FILE_CRITICAL],
                    kCriticalSuffix,
                    rotate_hours_);
        openLogFile(times_[FILE_SNAPSHOT], files_[FILE_SNAPSHOT],
                    kSnapshotSuffix,
                    rotate_hours_);
    } else {
        files_[FILE_NOTICE] = stdout;
        files_[FILE_ERROR] = stdout;
        files_[FILE_DEBUG] = stdout;
        files_[FILE_MONEY] = stdout;
        files_[FILE_HISTORY] = stdout;
        files_[FILE_INIT] = stdout;
        files_[FILE_SNAPSHOT] = stdout;
    }
    return this;
}

// IDA_EA: 0x854EC70
// IDA_NAME: _ZN4CLog11openLogFileE8TimeLog_RP8_IO_FILEPci
// DEMANGLED: CLog::openLogFile(TimeLog_,_IO_FILE *&,char *,int)
bool CLog::openLogFile(TimeLog_ time, FILE*& file, char* suffix,
                       int rotate_hours) {
    char name[4096];
    char path[4104];
    if (rotate_hours) {
        std::sprintf(name, "%s%04d%02d%02d-%02d", log_prefix_, time.year,
                     time.month, time.day, time.hour);
    } else {
        std::sprintf(name, "%s%04d%02d%02d", log_prefix_, time.year,
                     time.month, time.day);
    }
    std::strncpy(path, log_path_, 0x1000u);
    path[sizeof(path) - 1] = '\0';
    std::strncat(path, name, 0x1000u);
    std::strncat(path, suffix, 0x1000u);
    file = std::fopen(path, "ab");
    if (file == 0) {
        file = stdout;
        return true;
    }
    return false;
}

// IDA_EA: 0x854EDB0
// IDA_NAME: _ZN4CLog9logToFileERP8_IO_FILER8TimeLog_bPcPKcS5_b
// DEMANGLED: CLog::logToFile(_IO_FILE *&,TimeLog_ &,bool,char *,char const*,char *,bool)
int CLog::logToFile(FILE*& file, TimeLog_& previous, bool console,
                    char* suffix, const char* format, va_list args,
                    bool with_time) {
    if (this == 0 || file == 0) {
        return 0;
    }
    TimeLog_ now;
    now.setTime();
    if (file != stdout &&
        checkDay(now, &previous, file, suffix, rotate_hours_) != 1) {
        return 0;
    }

    char message[4096];
    char timed[4096];
    std::vsprintf(message, format, args);
    if (with_time) {
        std::sprintf(timed, "[%02d:%02d:%02d] %s", now.hour, now.minute,
                     now.second, message);
        std::fprintf(file, "%s\r\n", timed);
    } else {
        std::fprintf(file, "%s\r\n", message);
    }
    std::fflush(file);
    if (console) {
        if (with_time) {
            std::printf("%s\r\n", timed);
        } else {
            std::printf("%s\r\n", message);
        }
    }
    return 1;
}

// IDA_EA: 0x854EF56
// IDA_NAME: _ZN4CLog13logToInitFileERP8_IO_FILER8TimeLog_PcPKcS5_bb
// DEMANGLED: CLog::logToInitFile(_IO_FILE *&,TimeLog_ &,char *,char const*,char *,bool,bool)
int CLog::logToInitFile(FILE*& file, TimeLog_& previous, char* suffix,
                        const char* format, va_list args, bool with_time,
                        bool newline) {
    if (this == 0 || file == 0) {
        return 0;
    }
    TimeLog_ now;
    now.setTime();
    if (file != stdout &&
        checkDay(now, &previous, file, suffix, rotate_hours_) != 1) {
        return 0;
    }

    char raw[4096];
    char message[4096];
    std::vsprintf(raw, format, args);
    const char* printf_format = newline ? "%s\r\n" : "%s";
    std::printf(printf_format, raw);
    if (with_time) {
        std::sprintf(message, "[%02d:%02d:%02d] %s", now.hour, now.minute,
                     now.second, raw);
    } else {
        std::strcpy(message, raw);
    }
    std::fprintf(file, printf_format, message);
    std::fflush(file);
    return 1;
}

// IDA_EA: 0x854F594
// IDA_NAME: _ZN4CLog8checkDayE8TimeLog_PS0_RP8_IO_FILEPci
// DEMANGLED: CLog::checkDay(TimeLog_,TimeLog_*,_IO_FILE *&,char *,int)
int CLog::checkDay(TimeLog_ now, TimeLog_* previous, FILE*& file,
                   char* suffix, int rotate_hours) {
    if (previous == 0 || file == 0) {
        return 0;
    }
    bool reopen = false;
    if (rotate_hours) {
        reopen = previous->day != now.day ||
                 now.hour >= static_cast<unsigned char>(rotate_hours + previous->hour);
    } else {
        reopen = previous->day != now.day;
    }
    if (reopen) {
        *previous = now;
        std::fclose(file);
        openLogFile(now, file, suffix, rotate_hours);
    }
    return 1;
}

// IDA_EA: 0x854F692
// IDA_NAME: _ZN4CLog7getTimeEP8TimeLog_
// DEMANGLED: CLog::getTime(TimeLog_ *)
TimeLog_* CLog::getTime(TimeLog_* out) {
    if (out != 0) {
        out->setTime();
    }
    return out;
}

// IDA_EA: 0x854F162
// IDA_NAME: _ZN4CLog9logNoticeEPKcz
// DEMANGLED: CLog::logNotice(char const*,...)
int CLog::logNotice(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToFileV(FILE_NOTICE, false, kLogSuffix, format, args,
                            true);
    va_end(args);
    return result;
}

// IDA_EA: 0x854F1C2
// IDA_NAME: _ZN4CLog8logErrorEPKcz
// DEMANGLED: CLog::logError(char const*,...)
int CLog::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToFileV(FILE_ERROR, false, kErrorSuffix, format, args,
                            true);
    va_end(args);
    return result;
}

// IDA_EA: 0x854F222
// IDA_NAME: _ZN4CLog11logCriticalEPKcz
// DEMANGLED: CLog::logCritical(char const*,...)
int CLog::logCritical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToFileV(FILE_CRITICAL, false, kCriticalSuffix, format,
                            args, true);
    va_end(args);
    return result;
}

// IDA_EA: 0x854F282
// IDA_NAME: _ZN4CLog8logMoneyEPKcz
// DEMANGLED: CLog::logMoney(char const*,...)
int CLog::logMoney(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToFileV(FILE_MONEY, false, kMoneySuffix, format, args,
                            true);
    va_end(args);
    return result;
}

// IDA_EA: 0x854F2E2
// IDA_NAME: _ZN4CLog10logHistoryEPKcPc
// DEMANGLED: CLog::logHistory(char const*,char *)
int CLog::logHistory(const char* format, va_list args) {
    if (this == 0 || files_[FILE_HISTORY] == 0) {
        return 0;
    }
    if (files_[FILE_HISTORY] != stdout) {
        TimeLog_ now;
        now.setTime();
        if (checkDay(now, &times_[FILE_HISTORY], files_[FILE_HISTORY],
                     kHistorySuffix, history_rotate_hours_) != 1) {
            return 0;
        }
    }

    char message[4096];
    std::vsprintf(message, format, args);
    std::fprintf(files_[FILE_HISTORY], "%s\r\n", message);
    std::fflush(files_[FILE_HISTORY]);
    return 1;
}

// IDA_EA: 0x854F40A
// IDA_NAME: _ZN4CLog8logDebugEPKcz
// DEMANGLED: CLog::logDebug(char const*,...)
int CLog::logDebug(const char* format, ...) {
    (void)format;
    return 1;
}

// IDA_EA: 0x854F414
// IDA_NAME: _ZN4CLog10logConsoleEPKcz
// DEMANGLED: CLog::logConsole(char const*,...)
int CLog::logConsole(const char* format, ...) {
    if (this == 0) {
        return 0;
    }
    TimeLog_ now;
    now.setTime();
    char raw[4096];
    char message[4096];
    va_list args;
    va_start(args, format);
    std::vsprintf(raw, format, args);
    va_end(args);
    std::sprintf(message, "[%02d:%02d:%02d] %s", now.hour, now.minute,
                 now.second, raw);
    std::printf("%s\r\n", message);
    return 1;
}

// IDA_EA: 0x854F4C0
// IDA_NAME: _ZN4CLog11logSnapShotEPKcz
// DEMANGLED: CLog::logSnapShot(char const*,...)
int CLog::logSnapShot(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToFileV(FILE_SNAPSHOT, false, kSnapshotSuffix, format,
                            args, false);
    va_end(args);
    return result;
}

// IDA_EA: 0x854F520
// IDA_NAME: _ZN4CLog7logInitEbbPKcz
// DEMANGLED: CLog::logInit(bool,bool,char const*,...)
int CLog::logInit(bool with_time, bool newline, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = logToInitFile(files_[FILE_INIT], times_[FILE_INIT],
                               kInitSuffix, format, args, with_time, newline);
    va_end(args);
    return result;
}

bool CLog::initiated() const {
    return files_[FILE_NOTICE] != 0;
}

int CLog::logToFileV(LogFileIndex index, bool console, char* suffix,
                     const char* format, va_list args, bool with_time) {
    return logToFile(files_[index], times_[index], console, suffix, format,
                     args, with_time);
}

void CLog::closeFile(LogFileIndex index) {
    if (files_[index] != 0) {
        if (files_[index] != stdout) {
            std::fclose(files_[index]);
        }
        files_[index] = 0;
    }
}

// IDA_EA: 0x854E3DB
// IDA_NAME: _ZL17setLogWriterWiredv
// DEMANGLED: setLogWriterWired(void)
static int setLogWriterWired() {
    LogManager::setLogWriter(0, std::shared_ptr<LogWriter>(new LwServerWriter(2)));
    LogManager::setLogWriter(1, std::shared_ptr<LogWriter>(new LwServerWriter(1)));
    return 0;
}
