#ifndef OLDSRV_LOGGING_TRACE_H_
#define OLDSRV_LOGGING_TRACE_H_

#include "infra/concurrent/thread/mutex.h"

#include <stdint.h>

#include <array>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <string>


// IDA unit: 99_misc_global/LogWriter.
class LogWriter {
public:
    LogWriter();
    virtual ~LogWriter();

    virtual void log(const char* file, const char* function, int line,
                     const char* text) = 0;
};

// IDA unit: 99_misc_global/LwServerWriter.
class LwServerWriter : public LogWriter {
public:
    explicit LwServerWriter(int log_type);
    ~LwServerWriter();

    void log(const char* file, const char* function, int line,
             const char* text);

private:
    int log_type_;
};

// IDA unit: 99_misc_global/LwConsole.
class LwConsole : public LogWriter {
public:
    LwConsole();
    ~LwConsole();

    bool isSourcePositionVisible() const;
    LwConsole* setSourcePositionVisible(bool visible);
    void log(const char* file, const char* function, int line,
             const char* text);

private:
    bool source_position_visible_;
};

// IDA unit: 99_misc_global/LwCallback.
class LwCallback : public LogWriter {
public:
    typedef std::function<void(const char*, const char*, int, const char*)>
        Callback;

    LwCallback();
    explicit LwCallback(Callback callback);
    ~LwCallback();

    Callback getCallback() const;
    void setCallback(Callback callback);
    void log(const char* file, const char* function, int line,
             const char* text);

private:
    Callback callback_;
};

// IDA unit: 99_misc_global/CFLog.
class CFLog {
public:
    explicit CFLog(std::string& filename);
    virtual ~CFLog();

    int writeLog(std::tm* log_time, const char* format, va_list args);
    int writeLog(const char* format, va_list args);

private:
    FILE* file_;
    std::string filename_;
};

// IDA unit: 99_misc_global/CFileLogWriter.
class CFileLogWriter {
public:
    CFileLogWriter();
    virtual ~CFileLogWriter();

    int writeLog(const char* prefix, const char* format, ...);
    int writeRawLog(const char* prefix, const char* format, ...);

private:
    std::map<std::string, CFLog*> logs_;
    Mutex mutex_;
};

CFileLogWriter* CFileLogWriterInstance();

// IDA unit: 99_misc_global/CMyFileLog.
class CMyFileLog {
public:
    CMyFileLog(const char* function, int line);
    int operator()(const char* prefix, const char* format, ...);

private:
    const char* function_;
    int line_;
};

// IDA unit: 99_misc_global/CMyRawFileLog.
class CMyRawFileLog {
public:
    int operator()(const char* prefix, const char* format, ...);
};

// IDA unit: 99_misc_global/LogManagerData.
struct LogManagerData {
    LogManagerData();

    std::array<std::shared_ptr<LogWriter>, 64> writers;
};

// IDA unit: 99_misc_global/_LogManagerData_creater.
struct _LogManagerData_creater {
    _LogManagerData_creater();
};

// IDA unit: 99_misc_global/LogManager.
class LogManager {
public:
    // The restored tree keeps the C++ API at global scope, while the old
    // df_game_r binary exports these static methods as global LogManager names.
    static void logFormat(int log_index, const char* file,
                          const char* function, int line,
                          const char* format, ...);  // IDA_EA: 0x8AD3C0A
    static void logString(int log_index, const char* file,
                          const char* function, int line, const char* text);  // IDA_EA: 0x8AD3DA2
    static int pushIndent();  // IDA_EA: 0x8AD3EF6
    static int popIndent();  // IDA_EA: 0x8AD3F08
    static std::shared_ptr<LogWriter> getLogWriter(int log_index);  // IDA_EA: 0x8AD3F24
    static void setLogWriter(int log_index,
                             std::shared_ptr<LogWriter> writer);  // IDA_EA: 0x8AD3F64
};

// IDA unit: 99_misc_global/cMyTrace. IDA writes the first 14 bytes:
// function pointer, line, level, newline flag, with-time flag.  Four
// consecutive locals in CUser::make_basic_info have an exact 0x10 stride;
// the remaining two bytes are the compiler's natural tail padding.
class cMyTrace {
public:
    cMyTrace(const char* function, int line, int level);
    cMyTrace(const char* function, int line, int level, bool with_time,
             bool newline);
    int operator()(const char* format, ...);

private:
    const char* function_;
    int line_;
    int level_;
    bool newline_;
    bool with_time_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(cMyTrace) == 0x10u,
              "cMyTrace old-server ABI size must stay 0x10");
#endif

class cMyTraceNoop {
public:
    cMyTraceNoop();
    int operator()(const char* format = "", ...);

private:
    char reserved_;
};

// IDA unit: 99_misc_global/CSwitchLog.  The constructor at 0x854FD3C writes
// these four words in order; fair-PvP logging passes the object to the
// variadic call operator at 0x854FD64.
class CSwitchLog {
public:
    // IDA_EA: 0x854FD18
    // IDA_NAME: _ZN10CSwitchLog15SetLogWriteFlagEi
    // DEMANGLED: CSwitchLog::SetLogWriteFlag(int)
    static int SetLogWriteFlag(int enabled);

    CSwitchLog(const char* function, int line, int log_type,
               int log_write_index);
    int operator()(const char* format, ...);

private:
    const char* function_;  // +0x00
    int line_;              // +0x04
    int log_type_;          // +0x08
    int log_write_index_;   // +0x0c

    // The old binary stores four per-index enable bytes in one dword;
    // SetLogWriteFlag writes the complete value 0 or 1.
    static uint32_t log_write_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CSwitchLog) == 0x10u,
              "CSwitchLog IDA ABI size must stay 0x10");
#endif


#endif  // OLDSRV_LOGGING_TRACE_H_
