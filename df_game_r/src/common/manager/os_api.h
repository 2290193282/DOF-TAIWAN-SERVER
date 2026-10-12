#ifndef OLDSRV_RUNTIME_OS_API_H_
#define OLDSRV_RUNTIME_OS_API_H_

// IDA_EA: 0x86B1B48
// IDA_NAME: _Z9sleep_extii
// DEMANGLED: sleep_ext(int,int)
int sleep_ext(int seconds, int microseconds);

class OS_API {
public:
    // IDA_EA: 0x858C802
    // IDA_NAME: _ZN6OS_API15GetDateTimeTickEv
    // DEMANGLED: OS_API::GetDateTimeTick(void)
    static int GetDateTimeTick();

    // IDA_EA: 0x858C4E0
    // IDA_NAME: _ZN6OS_API5SleepEi
    // DEMANGLED: OS_API::Sleep(int)
    // The old server accepts milliseconds here and forwards milliseconds *
    // 1000 to usleep().  NetworkThread relies on this exact 10ms yield.
    static int Sleep(int milliseconds);

    // IDA_EA: 0x858C4FA
    // IDA_NAME: _ZN6OS_API6uSleepEj
    // DEMANGLED: OS_API::uSleep(unsigned int)
    static int uSleep(unsigned int microseconds);

    // IDA_EA: 0x858C53A
    // IDA_NAME: _ZN6OS_API4ExitEi
    // DEMANGLED: OS_API::Exit(int)
    static void Exit(int status) __attribute__((noreturn));

    // IDA_EA: 0x858C81C
    // IDA_NAME: _ZN6OS_API8snprintfEPcjPKcz
    // DEMANGLED: OS_API::snprintf(char *,unsigned int,char const*,...)
    static int snprintf(char* destination,
                        unsigned int capacity,
                        const char* format,
                        ...);
};

#endif  // OLDSRV_RUNTIME_OS_API_H_
