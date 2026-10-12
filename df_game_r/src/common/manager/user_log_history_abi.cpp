#include "infra/log/format/clog.h"
#include "infra/log/format/history_trace.h"
#include "infra/concurrent/thread/guard_mutex.h"

#include <cstdarg>


// IDA_EA: 0x854F990
// IDA_NAME: _ZN13cHistoryTraceclEPKcz
// DEMANGLED: cHistoryTrace::operator()(char const*,...)
int cHistoryTrace::operator()(const char* format, ...) {
    va_list args;
    va_start(args, format);
    CLog* log = CLog::instance();
    Guard<Mutex> guard(&log->mutexes_[7]);
    CLog::instance()->logHistory(format, args);
    va_end(args);
    return guard.release();
}
