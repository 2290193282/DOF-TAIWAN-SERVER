#include "old_data/channel/channel_timers.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/channel/arad_server_state.h"
#include "old_data/channel/log_game_channel.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <time.h>

// IDA_EA: 0x863B90E
// IDA_NAME: _ZN24Timer_RefreshServerState12dispatch_sigEiij
// DEMANGLED: Timer_RefreshServerState::dispatch_sig(int,int,unsigned int)
int Timer_RefreshServerState::dispatch_sig(int, int, unsigned int) {
    GlobalData::s_serverStateManager_->checkContinus();
    return 1;
}

// IDA_EA: 0x863D14A
// IDA_NAME: _ZN24Timer_RefreshServerStateC2Ev
// DEMANGLED: Timer_RefreshServerState::Timer_RefreshServerState(void)
Timer_RefreshServerState::Timer_RefreshServerState() : TimerSig() {}

// IDA_EA: 0x863E48A
// IDA_NAME: _ZN24Timer_RefreshServerStateD2Ev
// DEMANGLED: Timer_RefreshServerState::~Timer_RefreshServerState()
// IDA_EA: 0x863E4BA
// IDA_NAME: _ZN24Timer_RefreshServerStateD0Ev
// DEMANGLED: Timer_RefreshServerState::~Timer_RefreshServerState()
Timer_RefreshServerState::~Timer_RefreshServerState() {}

// IDA_EA: 0x8635A2C
// IDA_NAME: _ZN23TimerSaveLogGamechannel12dispatch_sigEiij
// DEMANGLED: TimerSaveLogGamechannel::dispatch_sig(int,int,unsigned int)
int TimerSaveLogGamechannel::dispatch_sig(int, int, unsigned int) {
    GlobalData::s_pLogGameChannel->ReqSaveLog();
    GlobalData::s_pLogGameChannel->ResetInOutUser();
    GlobalData::s_pLogGameChannel->SetNextLogTimer();
    return 1;
}

// IDA_EA: 0x863C672
// IDA_NAME: _ZN23TimerSaveLogGamechannelC2Ev
// DEMANGLED: TimerSaveLogGamechannel::TimerSaveLogGamechannel(void)
TimerSaveLogGamechannel::TimerSaveLogGamechannel() : TimerSig() {}

// IDA_EA: 0x8640188
// IDA_NAME: _ZN23TimerSaveLogGamechannelD2Ev
// DEMANGLED: TimerSaveLogGamechannel::~TimerSaveLogGamechannel()
// IDA_EA: 0x86401B8
// IDA_NAME: _ZN23TimerSaveLogGamechannelD0Ev
// DEMANGLED: TimerSaveLogGamechannel::~TimerSaveLogGamechannel()
TimerSaveLogGamechannel::~TimerSaveLogGamechannel() {}

// IDA_EA: 0x863575C
// IDA_NAME: _ZN32TimerStatisticsDetailChannelInfo12dispatch_sigEiij
// DEMANGLED: TimerStatisticsDetailChannelInfo::dispatch_sig(int,int,unsigned int)
int TimerStatisticsDetailChannelInfo::dispatch_sig(int, int, unsigned int) {
    G_GameWorld()->updateDetailChannelServiceInfo();
    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x8635782
// IDA_NAME: _ZN32TimerStatisticsDetailChannelInfo15RegistNextTimerEv
// DEMANGLED: TimerStatisticsDetailChannelInfo::RegistNextTimer(void)
int TimerStatisticsDetailChannelInfo::RegistNextTimer() {
    time_t current_time =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    struct tm local_time_value;
    struct tm* local_time = localtime_r(&current_time, &local_time_value);
    const int next_seconds =
        1800 - (local_time->tm_min % 30) * 60 - local_time->tm_sec;
    const int channel_no = G_CEnvironment()->get_channel_no();

    cMyTrace trace("bool TimerStatisticsDetailChannelInfo::RegistNextTimer()",
                   2348, 0);
    trace("content min:%d sec:%d next sec:%d  (channel no:%d)\n",
          local_time->tm_min, local_time->tm_sec, next_seconds, channel_no);

    G_TimerQueue()->InsertTimer(2, 0, 84, next_seconds, 0, 0);
    return 1;
}

// IDA_EA: 0x863C61E
// IDA_NAME: _ZN32TimerStatisticsDetailChannelInfoC2Ev
// DEMANGLED: TimerStatisticsDetailChannelInfo::TimerStatisticsDetailChannelInfo(void)
TimerStatisticsDetailChannelInfo::TimerStatisticsDetailChannelInfo()
    : TimerSig() {}

// IDA_EA: 0x8640224
// IDA_NAME: _ZN32TimerStatisticsDetailChannelInfoD2Ev
// DEMANGLED: TimerStatisticsDetailChannelInfo::~TimerStatisticsDetailChannelInfo()
// IDA_EA: 0x8640254
// IDA_NAME: _ZN32TimerStatisticsDetailChannelInfoD0Ev
// DEMANGLED: TimerStatisticsDetailChannelInfo::~TimerStatisticsDetailChannelInfo()
TimerStatisticsDetailChannelInfo::~TimerStatisticsDetailChannelInfo() {}
