#include "old_game/activity/schedule/online_preliminary_event.h"

#include "infra/log/format/trace.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"
#include "old_game/world/region/game_world.h"

namespace {

// Both StartEvent overloads and EndEvent perform the same original channel
// gate before toggling COnlinePreliminary's process-wide matchable flag.
void SetOnlinePreliminaryMatchableForCurrentChannel(bool matchable) {
    if (G_GameWorld()->GetChannelType() == 14) {
        online_preliminary::COnlinePreliminary::SetMatchAble(matchable);
    }
}

}  // namespace

// IDA_EA: 0x81BB490
// IDA_NAME: _ZN23COnlinePreliminaryEventC2Ev
// DEMANGLED: COnlinePreliminaryEvent::COnlinePreliminaryEvent(void)
COnlinePreliminaryEvent::COnlinePreliminaryEvent() : CEventBase() {}

// IDA_EA: 0x81BB4AC
// IDA_NAME: _ZN23COnlinePreliminaryEventD2Ev
// DEMANGLED: COnlinePreliminaryEvent::~COnlinePreliminaryEvent(void)
// IDA_EA: 0x81BB4DC
// IDA_NAME: _ZN23COnlinePreliminaryEventD0Ev
// DEMANGLED: COnlinePreliminaryEvent::~COnlinePreliminaryEvent(void)
COnlinePreliminaryEvent::~COnlinePreliminaryEvent() {}

// IDA_EA: 0x81BB4FA
// IDA_NAME: _ZN23COnlinePreliminaryEvent10StartEventEv
// DEMANGLED: COnlinePreliminaryEvent::StartEvent(void)
void COnlinePreliminaryEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void COnlinePreliminaryEvent::StartEvent()", 21,
                   0);
    trace("COnlinePreliminaryEvent Start!");
    SetOnlinePreliminaryMatchableForCurrentChannel(true);
}

// IDA_EA: 0x81BB56E
// IDA_NAME: _ZN23COnlinePreliminaryEvent10StartEventE10Word_Param
// DEMANGLED: COnlinePreliminaryEvent::StartEvent(Word_Param)
void COnlinePreliminaryEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void COnlinePreliminaryEvent::StartEvent(Word_Param)", 31,
        0);
    trace("COnlinePreliminaryEvent Start!");
    SetOnlinePreliminaryMatchableForCurrentChannel(true);
}

// IDA_EA: 0x81BB5E2
// IDA_NAME: _ZN23COnlinePreliminaryEvent8EndEventEv
// DEMANGLED: COnlinePreliminaryEvent::EndEvent(void)
void COnlinePreliminaryEvent::EndEvent() {
    SetEventFlag(false);
    SetOnlinePreliminaryMatchableForCurrentChannel(false);
}
