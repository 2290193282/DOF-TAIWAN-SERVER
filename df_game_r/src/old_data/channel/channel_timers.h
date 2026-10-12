#ifndef OLDSRV_OLD_DATA_CHANNEL_CHANNEL_TIMERS_H_
#define OLDSRV_OLD_DATA_CHANNEL_CHANNEL_TIMERS_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 03_channel_select/Timer_RefreshServerState.
class Timer_RefreshServerState : public TimerSig {
public:
    // IDA_EA: 0x863D14A
    // IDA_NAME: _ZN24Timer_RefreshServerStateC2Ev
    // DEMANGLED: Timer_RefreshServerState::Timer_RefreshServerState(void)
    Timer_RefreshServerState();

    // IDA_EA: 0x863B90E
    // IDA_NAME: _ZN24Timer_RefreshServerState12dispatch_sigEiij
    // DEMANGLED: Timer_RefreshServerState::dispatch_sig(int,int,unsigned int)
    int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863E48A (D2), 0x863E4BA (D0)
    // IDA_NAME: _ZN24Timer_RefreshServerStateD2Ev /
    //           _ZN24Timer_RefreshServerStateD0Ev
    // DEMANGLED: Timer_RefreshServerState::~Timer_RefreshServerState()
    virtual ~Timer_RefreshServerState();
};

// IDA unit: 03_channel_select/TimerSaveLogGamechannel.
class TimerSaveLogGamechannel : public TimerSig {
public:
    // IDA_EA: 0x863C672
    // IDA_NAME: _ZN23TimerSaveLogGamechannelC2Ev
    // DEMANGLED: TimerSaveLogGamechannel::TimerSaveLogGamechannel(void)
    TimerSaveLogGamechannel();

    // IDA_EA: 0x8635A2C
    // IDA_NAME: _ZN23TimerSaveLogGamechannel12dispatch_sigEiij
    // DEMANGLED: TimerSaveLogGamechannel::dispatch_sig(int,int,unsigned int)
    int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x8640188 (D2), 0x86401B8 (D0)
    // IDA_NAME: _ZN23TimerSaveLogGamechannelD2Ev /
    //           _ZN23TimerSaveLogGamechannelD0Ev
    // DEMANGLED: TimerSaveLogGamechannel::~TimerSaveLogGamechannel()
    virtual ~TimerSaveLogGamechannel();
};

// IDA unit: 03_channel_select/TimerStatisticsDetailChannelInfo.
class TimerStatisticsDetailChannelInfo : public TimerSig {
public:
    // IDA_EA: 0x863C61E
    // IDA_NAME: _ZN32TimerStatisticsDetailChannelInfoC2Ev
    // DEMANGLED: TimerStatisticsDetailChannelInfo::TimerStatisticsDetailChannelInfo(void)
    TimerStatisticsDetailChannelInfo();

    // IDA_EA: 0x863575C
    // IDA_NAME: _ZN32TimerStatisticsDetailChannelInfo12dispatch_sigEiij
    // DEMANGLED: TimerStatisticsDetailChannelInfo::dispatch_sig(int,int,unsigned int)
    int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x8635782
    // IDA_NAME: _ZN32TimerStatisticsDetailChannelInfo15RegistNextTimerEv
    // DEMANGLED: TimerStatisticsDetailChannelInfo::RegistNextTimer(void)
    int RegistNextTimer();

    // IDA_EA: 0x8640224 (D2), 0x8640254 (D0)
    // IDA_NAME: _ZN32TimerStatisticsDetailChannelInfoD2Ev /
    //           _ZN32TimerStatisticsDetailChannelInfoD0Ev
    // DEMANGLED: TimerStatisticsDetailChannelInfo::~TimerStatisticsDetailChannelInfo()
    virtual ~TimerStatisticsDetailChannelInfo();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Timer_RefreshServerState) == 0x04u,
              "Timer_RefreshServerState IDA ABI size must stay 0x04");
static_assert(sizeof(TimerSaveLogGamechannel) == 0x04u,
              "TimerSaveLogGamechannel IDA ABI size must stay 0x04");
static_assert(sizeof(TimerStatisticsDetailChannelInfo) == 0x04u,
              "TimerStatisticsDetailChannelInfo IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_DATA_CHANNEL_CHANNEL_TIMERS_H_
