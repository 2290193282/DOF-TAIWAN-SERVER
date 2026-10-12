#include "old_game/activity/schedule/timer_event_christmas.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/inter_server_event_start.h"

// IDA_EA: 0x863570A
// IDA_NAME: _ZN19TimerEventChristmas12dispatch_sigEiij
// DEMANGLED: TimerEventChristmas::dispatch_sig(int,int,unsigned int)
int TimerEventChristmas::dispatch_sig(int /*object_id*/,
                                      int exp_rate_delta,
                                      unsigned int event_id) {
    CServerEvent::SubExpRate(exp_rate_delta);
    CServerEvent::TurnOffEvent(static_cast<int>(event_id));
    const int event_interval =
        CServerEvent::GetEventInterval(static_cast<int>(event_id));
    Inter_MonitorServerEventStart::SendEventNotiPacket(
        false,
        static_cast<int16_t>(event_id),
        static_cast<int16_t>(event_interval),
        0);
    return 1;
}

// IDA_EA: 0x863C602 / 0x8640272 / 0x86402A2
TimerEventChristmas::TimerEventChristmas() : TimerSig() {}
TimerEventChristmas::~TimerEventChristmas() {}
