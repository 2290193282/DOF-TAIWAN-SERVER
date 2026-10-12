#include "common/manager/os_api.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <sys/select.h>
#include <unistd.h>

// IDA_EA: 0x858C802
// IDA_NAME: _ZN6OS_API15GetDateTimeTickEv
// DEMANGLED: OS_API::GetDateTimeTick(void)
int OS_API::GetDateTimeTick() {
    return GlobalData::system_time()->getCurSec();
}

// IDA_EA: 0x858C4E0
// IDA_NAME: _ZN6OS_API5SleepEi
// DEMANGLED: OS_API::Sleep(int)
int OS_API::Sleep(int milliseconds) {
    return ::usleep(1000 * milliseconds);
}

// IDA_EA: 0x858C4FA
// IDA_NAME: _ZN6OS_API6uSleepEj
// DEMANGLED: OS_API::uSleep(unsigned int)
int OS_API::uSleep(unsigned int microseconds) {
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = static_cast<suseconds_t>(microseconds);
    return ::select(0, 0, 0, 0, &timeout);
}

// IDA_EA: 0x858C53A
// IDA_NAME: _ZN6OS_API4ExitEi
// DEMANGLED: OS_API::Exit(int)
void OS_API::Exit(int status) {
    ::exit(status);
}

// IDA_EA: 0x86B1B48
// IDA_NAME: _Z9sleep_extii
// DEMANGLED: sleep_ext(int,int)
int sleep_ext(int seconds, int microseconds) {
    timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;
    return select(0, 0, 0, 0, &timeout);
}

// IDA_EA: 0x858C81C
// IDA_NAME: _ZN6OS_API8snprintfEPcjPKcz
// DEMANGLED: OS_API::snprintf(char *,unsigned int,char const*,...)
int OS_API::snprintf(char* destination,
                     unsigned int capacity,
                     const char* format,
                     ...) {
    va_list arguments;
    va_start(arguments, format);
    const int result = ::vsnprintf(destination, capacity, format, arguments);
    va_end(arguments);
    return result;
}
