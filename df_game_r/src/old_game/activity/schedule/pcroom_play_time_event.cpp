#include "old_game/activity/schedule/pcroom_play_time_event.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"

namespace {

const ENUM_PREMIUM_TYPE kPCRoomPremiumType =
    static_cast<ENUM_PREMIUM_TYPE>(12);

void ResetPCRoomPlayTime(CUser* user) {
    if (user && user->get_state() > 2 &&
        user->isAffectedPremium(kPCRoomPremiumType)) {
        user->setPcRoomPlayTimeData(0, 0, 0);
    }
}

void NotifyPCRoomPlayTime(CUser* user, unsigned int current_second) {
    if (user && user->get_state() > 2 &&
        user->isAffectedPremium(kPCRoomPremiumType)) {
        user->setPcRoomPlayStartTime(current_second);
        user->sendNotiPcRoomPlayTime();
    }
}

}  // namespace

// IDA_EA: 0x811C55A
// IDA_NAME: _ZN20CPCRoomPlayTimeEventC2Ev
// DEMANGLED: CPCRoomPlayTimeEvent::CPCRoomPlayTimeEvent(void)
CPCRoomPlayTimeEvent::CPCRoomPlayTimeEvent() : CEventBase() {}

// IDA_EA: 0x811C576
// IDA_NAME: _ZN20CPCRoomPlayTimeEventD2Ev
// DEMANGLED: CPCRoomPlayTimeEvent::~CPCRoomPlayTimeEvent(void)
// IDA_EA: 0x811C5A6
// IDA_NAME: _ZN20CPCRoomPlayTimeEventD0Ev
// DEMANGLED: CPCRoomPlayTimeEvent::~CPCRoomPlayTimeEvent(void)
CPCRoomPlayTimeEvent::~CPCRoomPlayTimeEvent() {}

// IDA_EA: 0x811C5C4
// IDA_NAME: _ZN20CPCRoomPlayTimeEvent10StartEventEv
// DEMANGLED: CPCRoomPlayTimeEvent::StartEvent(void)
void CPCRoomPlayTimeEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CPCRoomPlayTimeEvent::StartEvent()", 16, 0);
    trace("CPCRoomPlayTimeEvent Event Start!");
}

// IDA_EA: 0x811C616
// IDA_NAME: _ZN20CPCRoomPlayTimeEvent10StartEventE10Word_Param
// DEMANGLED: CPCRoomPlayTimeEvent::StartEvent(Word_Param)
void CPCRoomPlayTimeEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C630
// IDA_NAME: _ZN20CPCRoomPlayTimeEvent8EndEventEv
// DEMANGLED: CPCRoomPlayTimeEvent::EndEvent(void)
void CPCRoomPlayTimeEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CPCRoomPlayTimeEvent::EndEvent()", 31, 0);
    trace("CPCRoomPlayTimeEvent Event End!");
    G_GameWorld()->ProcessAllUsers(&ResetPCRoomPlayTime);
}

// IDA_EA: 0x811C754
// IDA_NAME: _ZN20CPCRoomPlayTimeEvent11StartActionEv
// DEMANGLED: CPCRoomPlayTimeEvent::StartAction(void)
void CPCRoomPlayTimeEvent::StartAction() {
    const unsigned int current_second = static_cast<unsigned int>(
        GlobalData::s_systemTime_.getCurSec());
    G_GameWorld()->ProcessAllUsers(
        [current_second](CUser* user) {
            NotifyPCRoomPlayTime(user, current_second);
        });
}
