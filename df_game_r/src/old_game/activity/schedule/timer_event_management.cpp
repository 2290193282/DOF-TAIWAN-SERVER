#include "old_game/activity/schedule/timer_event_management.h"
#include "infra/network/packet/PacketEnum.h"

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <ctime>

// The specialization is owned by game_world_event_staytime_runtime.cpp.  It
// must be declared here before the timer body uses it, otherwise this
// translation unit would instantiate the generic map walker a second time.
template <>
void GameWorld::ProcessAllUsers<CEventStayTime::RewardGoGoFighter>(
    CEventStayTime::RewardGoGoFighter reward);

// IDA_EA: 0x863AB92
// IDA_NAME: _ZN21Timer_EventManagement12dispatch_sigEiij
// DEMANGLED: Timer_EventManagement::dispatch_sig(int,int,unsigned int)
int Timer_EventManagement::dispatch_sig(int /*object_id*/,
                                        int /*param1*/,
                                        unsigned int /*param2*/) {
    CDataManager* const data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* const event_script_manager =
        data_manager->get_event_script_mng();
    event_script_manager->update_time_values(
        G_CEnvironment()->get_fatigue_time());
    RegistNextTimer(60);
    return 1;
}

// IDA_EA: 0x863ABD4
// IDA_NAME: _ZN21Timer_EventManagement15RegistNextTimerEi
// DEMANGLED: Timer_EventManagement::RegistNextTimer(int)
int Timer_EventManagement::RegistNextTimer(int delay_seconds) {
    return G_TimerQueue()->InsertTimer(2, 0, 156, delay_seconds, 0, 0);
}

// IDA_EA: 0x863D032
// IDA_NAME: _ZN21Timer_EventManagementC2Ev
// DEMANGLED: Timer_EventManagement::Timer_EventManagement(void)
Timer_EventManagement::Timer_EventManagement() : TimerSig() {}

// IDA_EA: 0x863E748 / 0x863E778
// IDA_NAME: _ZN21Timer_EventManagementD2Ev / _ZN21Timer_EventManagementD0Ev
// DEMANGLED: Timer_EventManagement::~Timer_EventManagement()
Timer_EventManagement::~Timer_EventManagement() {}

// IDA_EA: 0x863B928
// IDA_NAME: _ZN19Timer_StayTimeEvent12dispatch_sigEiij
// DEMANGLED: Timer_StayTimeEvent::dispatch_sig(int,int,unsigned int)
int Timer_StayTimeEvent::dispatch_sig(int /*object_id*/,
                                      int /*param1*/,
                                      unsigned int /*param2*/) {
    CEventStayTime* const repeat_event = static_cast<CEventStayTime*>(
        GlobalData::s_event_manager->GetRepeatEvent(159));
    if (repeat_event && repeat_event->IsEventing(0)) {
        const time_t check_time = time(0);
        CEventStayTime::RewardGoGoFighter reward(
            repeat_event->getRewardStayTime(),
            static_cast<long>(check_time));
        G_GameWorld()->ProcessAllUsers(reward);
        repeat_event->RegistNextTimer();
    }
    return 1;
}

// IDA_EA: 0x863D166
// IDA_NAME: _ZN19Timer_StayTimeEventC2Ev
// DEMANGLED: Timer_StayTimeEvent::Timer_StayTimeEvent(void)
Timer_StayTimeEvent::Timer_StayTimeEvent() : TimerSig() {}

// IDA_EA: 0x863E43C / 0x863E46C
// IDA_NAME: _ZN19Timer_StayTimeEventD2Ev / _ZN19Timer_StayTimeEventD0Ev
// DEMANGLED: Timer_StayTimeEvent::~Timer_StayTimeEvent()
Timer_StayTimeEvent::~Timer_StayTimeEvent() {}

// IDA_EA: 0x86364C4
// IDA_NAME: _ZN20TimerEventCoinRefill12dispatch_sigEiij
// DEMANGLED: TimerEventCoinRefill::dispatch_sig(int,int,unsigned int)
int TimerEventCoinRefill::dispatch_sig(int /*object_id*/,
                                       int /*param1*/,
                                       unsigned int /*param2*/) {
    // IDA intentionally dereferences this repeat-event slot without a null
    // guard.  Keep that lifecycle contract rather than silently changing the
    // event-manager failure behavior.
    CCoinRefillEvent* const repeat_event = static_cast<CCoinRefillEvent*>(
        GlobalData::s_event_manager->GetRepeatEvent(13));
    if (repeat_event->IsEventing(0)) {
        G_GameWorld()->ResetAllCoins();
        cMyTrace trace(
            "virtual bool TimerEventCoinRefill::dispatch_sig(int, int, memberIdentificationNumber_t)",
            2780,
            0);
        trace("Coin refilled by event!");
        repeat_event->registNextRefillTimer();
    }
    return 1;
}

// IDA_EA: 0x863C816
// IDA_NAME: _ZN20TimerEventCoinRefillC2Ev
// DEMANGLED: TimerEventCoinRefill::TimerEventCoinRefill(void)
TimerEventCoinRefill::TimerEventCoinRefill() : TimerSig() {}

// IDA_EA: 0x863FDE0 / 0x863FE10
// IDA_NAME: _ZN20TimerEventCoinRefillD2Ev / _ZN20TimerEventCoinRefillD0Ev
// DEMANGLED: TimerEventCoinRefill::~TimerEventCoinRefill()
TimerEventCoinRefill::~TimerEventCoinRefill() {}

// IDA_EA: 0x863B2FE
// IDA_NAME: _ZN42Timer_TournamentDugeonClearStateNotiReward12dispatch_sigEiij
// DEMANGLED: Timer_TournamentDugeonClearStateNotiReward::dispatch_sig(int,int,unsigned int)
int Timer_TournamentDugeonClearStateNotiReward::dispatch_sig(
    int party_index,
    int timer_key,
    unsigned int /*unused_param2*/) {
    // CParty::InsertTournamentDungeonTimer supplies party_index_ as the
    // TimerEntry object id and the generated key as param1.  The raw IDA
    // decompilation loses that implicit receiver argument of GetParty.
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party) {
        return 0;
    }
    if (!party->check_timer_key(
            TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_NOTI_REWARD,
            timer_key)) {
        return 0;
    }
    if (static_cast<unsigned char>(
            party->GetTournamentDungeonClearState()) == 1u) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT_STATE);
        packet.put_byte(1);
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party->get_user(seat);
            if (!user) {
                packet.put_byte(-1);
            } else if (static_cast<unsigned short>(
                           user->getCurCharacTotalFatigue()) ==
                       static_cast<unsigned short>(
                           user->getCurCharacTotalMaxFatigue())) {
                packet.put_byte(0);
            } else {
                packet.put_byte(1);
            }
        }
        // CMD455 has the same fixed eight-state body in the timeout path.
        for (int seat = 4; seat < 8; ++seat) {
            packet.put_byte(-1);
        }
        packet.finalize(true);
        party->send_to_party(packet);
        party->SetTournamentDungeonClearState(2);
        party->InsertTournamentDungeonTimer(
            TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT);
    }
    return 1;
}

// IDA_EA: 0x863D0BE
// IDA_NAME: _ZN42Timer_TournamentDugeonClearStateNotiRewardC2Ev
// DEMANGLED: Timer_TournamentDugeonClearStateNotiReward::Timer_TournamentDugeonClearStateNotiReward(void)
Timer_TournamentDugeonClearStateNotiReward::
    Timer_TournamentDugeonClearStateNotiReward()
    : TimerSig() {}

// IDA_EA: 0x863E5C2 / 0x863E5F2
// IDA_NAME: _ZN42Timer_TournamentDugeonClearStateNotiRewardD2Ev / _ZN42Timer_TournamentDugeonClearStateNotiRewardD0Ev
// DEMANGLED: Timer_TournamentDugeonClearStateNotiReward::~Timer_TournamentDugeonClearStateNotiReward()
Timer_TournamentDugeonClearStateNotiReward::
    ~Timer_TournamentDugeonClearStateNotiReward() {}
