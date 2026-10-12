#include "infra/log/format/trace.h"

#include "infra/concurrent/thread/guard_mutex.h"
#include "infra/log/format/clog.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>


namespace {

const int kTraceBufferSize = 4096;

int s_indent = 0;
LogManagerData* s_data = 0;

}  // namespace

// IDA_EA: 0x8AD3990
// IDA_NAME: _ZN9LogWriterC2Ev
// DEMANGLED: LogWriter::LogWriter(void)
LogWriter::LogWriter() {}

// IDA_EA: 0x8AD399E
// IDA_NAME: _ZN9LogWriterD2Ev
// DEMANGLED: LogWriter::~LogWriter()
LogWriter::~LogWriter() {}

// IDA_EA: 0x854E2AE
// IDA_NAME: _ZN14LwServerWriterC2E13ENUM_LOG_TYPE
// DEMANGLED: LwServerWriter::LwServerWriter(ENUM_LOG_TYPE)
LwServerWriter::LwServerWriter(int log_type) : log_type_(log_type) {}

// IDA_EA: 0x8550124
// IDA_NAME: _ZN14LwServerWriterD2Ev
// DEMANGLED: LwServerWriter::~LwServerWriter()
LwServerWriter::~LwServerWriter() {}

// IDA_EA: 0x854E2D4
// IDA_NAME: _ZN14LwServerWriter3logEPKcS1_iS1_
// DEMANGLED: LwServerWriter::log(char const*,char const*,int,char const*)
void LwServerWriter::log(const char* file,
                         const char* function,
                         int line,
                         const char* text) {
    char message[kTraceBufferSize];
    if (function != 0) {
        std::snprintf(message, sizeof(message), "%s(%d): %s", function, line,
                      text);
    } else {
        std::snprintf(message, sizeof(message), "%s", text);
    }
    message[sizeof(message) - 1] = '\0';

    CLog* log = CLog::instance();
    if (log == 0) {
        std::printf("%s\r\n", message);
    } else if (log_type_ == 1) {
        log->logError("%s", message);
    } else if (log_type_ == 2) {
        log->logDebug("%s", message);
    }
}

// IDA_EA: 0x8AD39E2
// IDA_NAME: _ZN9LwConsoleC2Ev
// DEMANGLED: LwConsole::LwConsole(void)
LwConsole::LwConsole() : source_position_visible_(false) {}

// IDA_EA: 0x8AD44FA
// IDA_NAME: _ZN9LwConsoleD2Ev
// DEMANGLED: LwConsole::~LwConsole()
LwConsole::~LwConsole() {}

// IDA_EA: 0x8AD3A06
// IDA_NAME: _ZNK9LwConsole23isSourcePositionVisibleEv
// DEMANGLED: LwConsole::isSourcePositionVisible(void)const
bool LwConsole::isSourcePositionVisible() const {
    return source_position_visible_;
}

// IDA_EA: 0x8AD3A12
// IDA_NAME: _ZN9LwConsole24setSourcePositionVisibleEb
// DEMANGLED: LwConsole::setSourcePositionVisible(bool)
LwConsole* LwConsole::setSourcePositionVisible(bool visible) {
    source_position_visible_ = visible;
    return this;
}

// IDA_EA: 0x8AD3A2A
// IDA_NAME: _ZN9LwConsole3logEPKcS1_iS1_
// DEMANGLED: LwConsole::log(char const*,char const*,int,char const*)
void LwConsole::log(const char* file,
                    const char* function,
                    int line,
                    const char* text) {
    if (source_position_visible_) {
        std::printf("%s(%d) %s: ", file, line, function);
    }
    std::puts(text);
}

// IDA_EA: 0x8AD3A6A
// IDA_NAME: _ZN10LwCallbackC2Ev
// DEMANGLED: LwCallback::LwCallback(void)
LwCallback::LwCallback() : callback_() {}

// IDA_EA: 0x8AD3AB8
// IDA_NAME: _ZN10LwCallbackC2ESt8functionIFvPKcS2_iS2_EE
// DEMANGLED: LwCallback::LwCallback(std::function<void(char const*,char const*,int,char const*)>)
LwCallback::LwCallback(Callback callback) : callback_(callback) {}

// IDA_EA: 0x8AD447A
// IDA_NAME: _ZN10LwCallbackD2Ev
// DEMANGLED: LwCallback::~LwCallback()
LwCallback::~LwCallback() {}

// IDA_EA: 0x8AD3B0E
// IDA_NAME: _ZNK10LwCallback11getCallbackEv
// DEMANGLED: LwCallback::getCallback(void)const
LwCallback::Callback LwCallback::getCallback() const {
    return callback_;
}

// IDA_EA: 0x8AD3B38
// IDA_NAME: _ZN10LwCallback11setCallbackESt8functionIFvPKcS2_iS2_EE
// DEMANGLED: LwCallback::setCallback(std::function<void(char const*,char const*,int,char const*)>)
void LwCallback::setCallback(Callback callback) {
    callback_ = callback;
}

// IDA_EA: 0x8AD3B56
// IDA_NAME: _ZN10LwCallback3logEPKcS1_iS1_
// DEMANGLED: LwCallback::log(char const*,char const*,int,char const*)
void LwCallback::log(const char* file,
                     const char* function,
                     int line,
                     const char* text) {
    if (callback_) {
        callback_(file, function, line, text);
    }
}

uint32_t CSwitchLog::log_write_ = 0;

// IDA_EA: 0x854FD18
// IDA_NAME: _ZN10CSwitchLog15SetLogWriteFlagEi
// DEMANGLED: CSwitchLog::SetLogWriteFlag(int)
int CSwitchLog::SetLogWriteFlag(int enabled) {
    if (enabled == 0) {
        log_write_ = 0;
    } else if (enabled == 1) {
        log_write_ = 1;
    }
    return enabled;
}

// IDA_EA: 0x854FD3C
// IDA_NAME: _ZN10CSwitchLogC2EPKciii
// DEMANGLED: CSwitchLog::CSwitchLog(char const*,int,int,int)
CSwitchLog::CSwitchLog(const char* function,
                       int line,
                       int log_type,
                       int log_write_index)
    : function_(function),
      line_(line),
      log_type_(log_type),
      log_write_index_(log_write_index) {}

// IDA_EA: 0x854FD64
// IDA_NAME: _ZN10CSwitchLogclEPKcz
// DEMANGLED: CSwitchLog::operator()(char const*,...)
int CSwitchLog::operator()(const char* format, ...) {
    const uint8_t enabled =
        reinterpret_cast<const uint8_t*>(&log_write_)[log_write_index_];
    if (!enabled) {
        return enabled;
    }

    char text[4096];
    std::sprintf(text, "%s(%d): ", function_, line_);

    va_list args;
    va_start(args, format);
    const size_t prefix_length = std::strlen(text);
    std::vsnprintf(text + prefix_length,
                   4095u - prefix_length,
                   format,
                   args);
    va_end(args);

    CLog* const log = CLog::instance();
    MutexGuard guard(&log->mutexes_[log_type_]);
    switch (log_type_) {
        case 0:
            return log->logNotice(text);
        case 1:
            return log->logError(text);
        case 2:
            return log->logDebug(text);
        case 3:
            return log->logConsole(text);
        case 4:
            return log->logMoney(text);
        case 5:
            return log->logCritical(text);
        default:
            return 0;
    }
}

// IDA_EA: 0x8106DC4
// IDA_NAME: _ZN5CFLogC2ERSs
// DEMANGLED: CFLog::CFLog(std::string &)
CFLog::CFLog(std::string& filename) : file_(0), filename_(filename) {
    file_ = std::fopen(filename.c_str(), "ab");
    if (file_ == 0) {
        file_ = stdout;
    }
}

// IDA_EA: 0x8106E5C
// IDA_NAME: _ZN5CFLogD2Ev
// DEMANGLED: CFLog::~CFLog()
// IDA deleting destructor: 0x8106EC4.
CFLog::~CFLog() {
    if (file_ != 0) {
        std::fclose(file_);
    }
}

// IDA_EA: 0x8106EE2
// IDA_NAME: _ZN5CFLog8writeLogEP2tmPKcPc
// DEMANGLED: CFLog::writeLog(tm *,char const*,char *)
int CFLog::writeLog(std::tm* log_time, const char* format, va_list args) {
    char formatted[28680];
    char line[28672];

    if (file_ == 0) {
        return 0;
    }

    std::vsprintf(formatted, format, args);
    std::sprintf(line, "[%02d:%02d:%02d] %s", log_time->tm_hour,
                 log_time->tm_min, log_time->tm_sec, formatted);
    std::fprintf(file_, "%s\r\n", line);
    return std::fflush(file_);
}

// IDA_EA: 0x8106F8E
// IDA_NAME: _ZN5CFLog8writeLogEPKcPc
// DEMANGLED: CFLog::writeLog(char const*,char *)
int CFLog::writeLog(const char* format, va_list args) {
    char formatted[28680];

    if (file_ == 0) {
        return 0;
    }

    std::vsprintf(formatted, format, args);
    std::fprintf(file_, "%s\r\n", formatted);
    return std::fflush(file_);
}

// IDA_EA: 0x8106FF0
// IDA_NAME: _ZN14CFileLogWriterC2Ev
// DEMANGLED: CFileLogWriter::CFileLogWriter(void)
CFileLogWriter::CFileLogWriter() : logs_(), mutex_() {}

// IDA_EA: 0x810701E
// IDA_NAME: _ZN14CFileLogWriterD2Ev
// DEMANGLED: CFileLogWriter::~CFileLogWriter()
// IDA deleting destructor: 0x8107060.
CFileLogWriter::~CFileLogWriter() {}

// IDA_EA: 0x810707E
// IDA_NAME: _ZN14CFileLogWriter8writeLogEPKcS1_z
// DEMANGLED: CFileLogWriter::writeLog(char const*,char const*,...)
int CFileLogWriter::writeLog(const char* prefix, const char* format, ...) {
    char yesterday_path[256];
    char today_path[256];
    std::tm yesterday_tm;
    std::tm today_tm;
    va_list args;
    va_start(args, format);

    MutexGuard guard(&mutex_);
    const time_t now = std::time(0);
    std::tm* current = localtime_r(&now, &today_tm);
    va_list copied_args;
    va_copy(copied_args, args);

    std::sprintf(today_path, "%s%04d%02d%02d.log", prefix,
                 current->tm_year + 1900, current->tm_mon + 1,
                 current->tm_mday);
    std::string today_name(today_path);

    std::map<std::string, CFLog*>::iterator current_log =
        logs_.find(today_name);
    if (current_log != logs_.end()) {
        current_log->second->writeLog(current, format, copied_args);
    } else {
        const time_t yesterday = now - 86400;
        std::tm* previous = localtime_r(&yesterday, &yesterday_tm);
        std::sprintf(yesterday_path, "%s%04d%02d%02d.log", prefix,
                     previous->tm_year + 1900, previous->tm_mon + 1,
                     previous->tm_mday);
        std::string yesterday_name(yesterday_path);

        std::map<std::string, CFLog*>::iterator previous_log =
            logs_.find(yesterday_name);
        if (previous_log != logs_.end()) {
            if (previous_log->second != 0) {
                delete previous_log->second;
            }
            logs_.erase(previous_log);
        }

        CFLog* created_log = new CFLog(today_name);
        std::pair<std::map<std::string, CFLog*>::iterator, bool> inserted =
            logs_.insert(std::make_pair(today_name, created_log));
        if (inserted.second) {
            created_log->writeLog(current, format, copied_args);
        }
    }

    va_end(copied_args);
    va_end(args);
    return 0;
}

// IDA_EA: 0x810747C
// IDA_NAME: _ZN14CFileLogWriter11writeRawLogEPKcS1_z
// DEMANGLED: CFileLogWriter::writeRawLog(char const*,char const*,...)
int CFileLogWriter::writeRawLog(const char* prefix,
                                const char* format,
                                ...) {
    char yesterday_path[256];
    char today_path[256];
    std::tm yesterday_tm;
    std::tm today_tm;
    va_list args;
    va_start(args, format);

    MutexGuard guard(&mutex_);
    const time_t now = std::time(0);
    std::tm* current = localtime_r(&now, &today_tm);
    va_list copied_args;
    va_copy(copied_args, args);

    std::sprintf(today_path, "%s%04d%02d%02d.log", prefix,
                 current->tm_year + 1900, current->tm_mon + 1,
                 current->tm_mday);
    std::string today_name(today_path);

    std::map<std::string, CFLog*>::iterator current_log =
        logs_.find(today_name);
    if (current_log != logs_.end()) {
        current_log->second->writeLog(format, copied_args);
    } else {
        const time_t yesterday = now - 86400;
        std::tm* previous = localtime_r(&yesterday, &yesterday_tm);
        std::sprintf(yesterday_path, "%s%04d%02d%02d.log", prefix,
                     previous->tm_year + 1900, previous->tm_mon + 1,
                     previous->tm_mday);
        std::string yesterday_name(yesterday_path);

        std::map<std::string, CFLog*>::iterator previous_log =
            logs_.find(yesterday_name);
        if (previous_log != logs_.end()) {
            if (previous_log->second != 0) {
                delete previous_log->second;
            }
            logs_.erase(previous_log);
        }

        CFLog* created_log = new CFLog(today_name);
        std::pair<std::map<std::string, CFLog*>::iterator, bool> inserted =
            logs_.insert(std::make_pair(today_name, created_log));
        if (inserted.second) {
            created_log->writeLog(format, copied_args);
        }
    }

    va_end(copied_args);
    va_end(args);
    return 0;
}

// IDA_EA: 0x8106D30
// IDA_NAME: _Z22CFileLogWriterInstancev
// DEMANGLED: CFileLogWriterInstance(void)
CFileLogWriter* CFileLogWriterInstance() {
    static CFileLogWriter instance;
    return &instance;
}

// IDA_EA: 0x810786C
// IDA_NAME: _ZN10CMyFileLogC2EPKci
// DEMANGLED: CMyFileLog::CMyFileLog(char const*,int)
CMyFileLog::CMyFileLog(const char* function, int line)
    : function_(function), line_(line) {}

// IDA_EA: 0x8107898
// IDA_NAME: _ZN10CMyFileLogclEPKcS1_z
// DEMANGLED: CMyFileLog::operator()(char const*,char const*,...)
int CMyFileLog::operator()(const char* prefix, const char* format, ...) {
    char text[28672];
    std::memset(text, 0, sizeof(text));
    std::sprintf(text, "%s(%d): ", function_, line_);

    va_list args;
    va_start(args, format);
    va_list copied_args;
    va_copy(copied_args, args);
    const size_t offset = std::strlen(text);
    std::vsprintf(&text[offset], format, args);
    va_end(copied_args);
    va_end(args);

    CFileLogWriter* writer = CFileLogWriterInstance();
    return writer->writeLog(prefix, text);
}

// IDA_EA: 0x8107A90
// IDA_NAME: _ZN13CMyRawFileLogclEPKcS1_z
// DEMANGLED: CMyRawFileLog::operator()(char const*,char const*,...)
int CMyRawFileLog::operator()(const char* prefix, const char* format, ...) {
    char text[28672];
    std::memset(text, 0, sizeof(text));

    va_list args;
    va_start(args, format);
    va_list copied_args;
    va_copy(copied_args, args);
    std::vsprintf(text, format, args);
    va_end(copied_args);
    va_end(args);

    CFileLogWriter* writer = CFileLogWriterInstance();
    return writer->writeRawLog(prefix, text);
}

// IDA_EA: 0x8AD40A4
// IDA_NAME: _ZN14LogManagerDataC2Ev
// DEMANGLED: LogManagerData::LogManagerData(void)
LogManagerData::LogManagerData() : writers() {}

// IDA_EA: 0x8AD3BA0
// IDA_NAME: _ZL11prepareDatav
// DEMANGLED: prepareData(void)
static LogManagerData* prepareData() {
    if (s_data == 0) {
        s_data = new LogManagerData();
    }
    return s_data;
}

// IDA_EA: 0x8AD40B8
// IDA_NAME: _ZN23_LogManagerData_createrC2Ev
// DEMANGLED: _LogManagerData_creater::_LogManagerData_creater(void)
_LogManagerData_creater::_LogManagerData_creater() {
    prepareData();
}

// IDA_EA: 0x8AD3C0A
// IDA_NAME: _ZN10LogManager9logFormatEiPKcS1_iS1_z
// DEMANGLED: LogManager::logFormat(int,char const*,char const*,int,char const*,...)
void LogManager::logFormat(int log_index,
                           const char* file,
                           const char* function,
                           int line,
                           const char* format,
                           ...) {
    char message[16384];
    va_list args;
    va_start(args, format);
    std::vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    message[sizeof(message) - 1] = '\0';

    LogManager::logString(log_index, file, function, line, message);
}

// IDA_EA: 0x8AD3DA2
// IDA_NAME: _ZN10LogManager9logStringEiPKcS1_iS1_
// DEMANGLED: LogManager::logString(int,char const*,char const*,int,char const*)
void LogManager::logString(int log_index,
                           const char* file,
                           const char* function,
                           int line,
                           const char* text) {
    LogManagerData* data = prepareData();
    std::shared_ptr<LogWriter> writer = data->writers[log_index];
    if (!writer) {
        return;
    }

    if (s_indent <= 0) {
        writer->log(file, function, line, text);
        return;
    }

    std::string indented;
    for (int count = 0; count < s_indent; ++count) {
        indented += "\t";
    }
    indented += text;
    writer->log(file, function, line, indented.c_str());
}

// IDA_EA: 0x8AD3EF6
// IDA_NAME: _ZN10LogManager10pushIndentEv
// DEMANGLED: LogManager::pushIndent(void)
int LogManager::pushIndent() {
    return ++s_indent;
}

// IDA_EA: 0x8AD3F08
// IDA_NAME: _ZN10LogManager9popIndentEv
// DEMANGLED: LogManager::popIndent(void)
int LogManager::popIndent() {
    if (s_indent > 0) {
        return --s_indent;
    }
    return s_indent;
}

// IDA_EA: 0x8AD3F24
// IDA_NAME: _ZN10LogManager12getLogWriterEi
// DEMANGLED: LogManager::getLogWriter(int)
std::shared_ptr<LogWriter> LogManager::getLogWriter(int log_index) {
    LogManagerData* data = prepareData();
    return data->writers[log_index];
}

// IDA_EA: 0x8AD3F64
// IDA_NAME: _ZN10LogManager12setLogWriterEiSt10shared_ptrI9LogWriterE
// DEMANGLED: LogManager::setLogWriter(int,std::shared_ptr<LogWriter>)
void LogManager::setLogWriter(int log_index,
                              std::shared_ptr<LogWriter> writer) {
    LogManagerData* data = prepareData();
    data->writers[log_index] = writer;
}

// IDA_EA: 0x854F718
// IDA_NAME: _ZN8cMyTraceC2EPKcii
// DEMANGLED: cMyTrace::cMyTrace(char const*,int,int)
cMyTrace::cMyTrace(const char* function, int line, int level)
    : function_(function),
      line_(line),
      level_(level),
      newline_(true),
      with_time_(false) {}

// IDA_EA: 0x854F746
// IDA_NAME: _ZN8cMyTraceC2EPKciibb
// DEMANGLED: cMyTrace::cMyTrace(char const*,int,int,bool,bool)
cMyTrace::cMyTrace(const char* function,
                   int line,
                   int level,
                   bool with_time,
                   bool newline)
    : function_(function),
      line_(line),
      level_(level),
      newline_(newline),
      with_time_(with_time) {}

// IDA_EA: 0x854F788
// IDA_NAME: _ZN8cMyTraceclEPKcz
// DEMANGLED: cMyTrace::operator()(char const*,...)
int cMyTrace::operator()(const char* format, ...) {
    char text[kTraceBufferSize];
    std::memset(text, 0, sizeof(text));
    if (level_ != 8 && level_ != 9) {
        std::sprintf(text, "%s(%d): ", function_, line_);
    }

    va_list args;
    va_start(args, format);
    const unsigned int offset =
        static_cast<unsigned int>(std::strlen(text));
    std::vsnprintf(text + offset, 4095u - offset, format, args);
    va_end(args);

    CLog* log = CLog::instance();
    Guard<Mutex> guard(&log->mutexes_[level_]);

    switch (level_) {
        case 0:
            CLog::instance()->logNotice(text);
            break;
        case 1:
            CLog::instance()->logError(text);
            break;
        case 2:
            CLog::instance()->logDebug(text);
            break;
        case 3:
            CLog::instance()->logConsole(text);
            break;
        case 4:
            CLog::instance()->logMoney(text);
            break;
        case 5:
            CLog::instance()->logCritical(text);
            break;
        case 8:
            CLog::instance()->logSnapShot(text);
            CLog::instance()->logInit(with_time_, newline_, text);
            break;
        case 9:
            CLog::instance()->logInit(with_time_, newline_, text);
            break;
        default:
            break;
    }
    return guard.release();
}

// IDA_EA: 0x822AD5A
// IDA_NAME: _ZN12cMyTraceNoopC2Ev
// DEMANGLED: cMyTraceNoop::cMyTraceNoop(void)
cMyTraceNoop::cMyTraceNoop() : reserved_(0) {}

// IDA_EA: 0x822AD60
// IDA_NAME: _ZN12cMyTraceNoopclEPKcz
// DEMANGLED: cMyTraceNoop::operator()(char const*,...)
int cMyTraceNoop::operator()(const char* format, ...) {
    (void)format;
    return 0;
}
