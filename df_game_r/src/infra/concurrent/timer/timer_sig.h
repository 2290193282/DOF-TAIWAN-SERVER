#ifndef OLDSRV_RUNTIME_TIMER_SIG_H_
#define OLDSRV_RUNTIME_TIMER_SIG_H_


// IDA vtable: off_8B694F0 at 0x8B694F0.
// Slot +0x00 is the pure TimerSig::dispatch_sig(int,int,unsigned int)
// contract; +0x04/+0x08 are D2/D0.
class TimerSig {
public:
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int param2) = 0;

    // IDA_EA: 0x812C9E2 (D2), 0x812CA08 (D0)
    // IDA_NAME: _ZN8TimerSigD2Ev / _ZN8TimerSigD0Ev
    // DEMANGLED: TimerSig::~TimerSig()
    virtual ~TimerSig();

protected:
    // IDA_EA: 0x863BB30
    // IDA_NAME: _ZN8TimerSigC2Ev
    // DEMANGLED: TimerSig::TimerSig(void)
    TimerSig();
};


#endif  // OLDSRV_RUNTIME_TIMER_SIG_H_
