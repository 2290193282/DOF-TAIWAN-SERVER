#ifndef OLDSRV_LOGGING_CLOG_H_
#define OLDSRV_LOGGING_CLOG_H_

#include <cstdarg>
#include <cstdio>
#include <string>

#include "infra/concurrent/thread/mutex.h"


class cHistoryTrace;
class cMyTrace;


// IDA: TimeLog_ @0x854E228. The old layout is 8 bytes:
// u16 year, u8 month/day/hour/min/sec, plus one padding byte.
struct TimeLog_ {
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char reserved;

    // IDA: TimeLog_::setTime @0x854E228.
    TimeLog_* setTime();
};

static_assert(sizeof(TimeLog_) == 0x08u,
              "TimeLog_ old-server ABI size must stay 0x08");

// IDA: CLog singleton, object allocation size 0x358 at createInstance.
class CLog {
public:
    CLog();
    ~CLog();

    // IDA: CLog::createInstance @0x82A3CB3.
    static CLog* createInstance();

    // IDA: CLog::instance @0x817DFEC.
    static CLog* instance();

    // IDA: CLog::destroyInstance @0x82A3CFD.
    static void destroyInstance();

    // IDA: CLog::initiate @0x854E88C.
    CLog* initiate(char* path, char* src, int rotate_hours);

    // IDA: CLog::openLogFile @0x854EC70.
    bool openLogFile(TimeLog_ time, FILE*& file, char* suffix,
                     int rotate_hours);

    // IDA: CLog::logToFile @0x854EDB0.
    int logToFile(FILE*& file, TimeLog_& time, bool console,
                  char* suffix, const char* format, va_list args,
                  bool with_time);

    // IDA: CLog::logToInitFile @0x854EF56.
    int logToInitFile(FILE*& file, TimeLog_& time, char* suffix,
                      const char* format, va_list args, bool with_time,
                      bool newline);

    // IDA: CLog::checkDay @0x854F594.
    int checkDay(TimeLog_ now, TimeLog_* previous, FILE*& file,
                 char* suffix, int rotate_hours);

    // IDA: CLog::getTime @0x854F692.
    TimeLog_* getTime(TimeLog_* out);

    int logNotice(const char* format, ...);    // IDA @0x854F162.
    int logError(const char* format, ...);     // IDA @0x854F1C2.
    int logCritical(const char* format, ...);  // IDA @0x854F222.
    int logMoney(const char* format, ...);     // IDA @0x854F282.
    int logHistory(const char* format, va_list args);  // IDA @0x854F2E2.
    int logDebug(const char* format, ...);     // IDA @0x854F40A returns 1.
    int logConsole(const char* format, ...);   // IDA @0x854F414.
    int logSnapShot(const char* format, ...);  // IDA @0x854F4C0.
    int logInit(bool with_time, bool newline, const char* format, ...);

    bool initiated() const;

private:
    friend class cHistoryTrace;
    friend class cMyTrace;
    friend class CSwitchLog;

    enum LogFileIndex {
        FILE_NOTICE = 0,
        FILE_ERROR = 1,
        FILE_DEBUG = 2,
        FILE_MONEY = 3,
        FILE_HISTORY = 4,
        FILE_CRITICAL = 5,
        FILE_INIT = 6,
        FILE_SNAPSHOT = 7,
        FILE_COUNT = 8,
    };

    int logToFileV(LogFileIndex index, bool console, char* suffix,
                   const char* format, va_list args, bool with_time);
    void closeFile(LogFileIndex index);

    static CLog* this_;

    // Old 32-bit CLog storage, verified from CLog::CLog @0x854E558 and
    // CLog::initiate @0x854E88C.  cHistoryTrace @0x854F990 locks index 7.
    Mutex mutexes_[10];              // +0x000, 10 * 0x18.
    FILE* files_[FILE_COUNT];        // +0x0F0.
    char log_path_[256];             // +0x110.
    TimeLog_ times_[FILE_COUNT];     // +0x210.
    int rotate_hours_;               // +0x250.
    int history_rotate_hours_;       // +0x254.
    char log_prefix_[256];           // +0x258, sizeof(CLog) == 0x358.
};


#endif  // OLDSRV_LOGGING_CLOG_H_
