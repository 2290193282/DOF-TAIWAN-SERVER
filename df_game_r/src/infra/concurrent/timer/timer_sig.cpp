#include "infra/concurrent/timer/timer_sig.h"


#if defined(__i386__)
typedef char TimerSig_Size_Must_Be_4[(sizeof(TimerSig) == 4) ? 1 : -1];
#endif


// IDA_EA: 0x863BB30
// IDA_NAME: _ZN8TimerSigC2Ev
// DEMANGLED: TimerSig::TimerSig(void)
TimerSig::TimerSig() {}

// IDA_EA: 0x812C9E2
// IDA_NAME: _ZN8TimerSigD2Ev
// DEMANGLED: TimerSig::~TimerSig()
// IDA_EA: 0x812CA08
// IDA_NAME: _ZN8TimerSigD0Ev
// DEMANGLED: TimerSig::~TimerSig()
TimerSig::~TimerSig() {}
