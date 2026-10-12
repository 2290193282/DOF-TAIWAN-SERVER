#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/log/format/log_manager.h"
#include "old_game/player/core/cuser.h"

namespace advancealtar {

// IDA_EA: 0x863D112
// IDA_NAME: _ZN12advancealtar14Timer_MainTickC2Ev
Timer_MainTick::Timer_MainTick() : TimerSig() {}

// IDA_EA: 0x813DBE4 / 0x813DC14
// IDA_NAME: _ZN12advancealtar14Timer_MainTickD2Ev / _ZN12advancealtar14Timer_MainTickD0Ev
Timer_MainTick::~Timer_MainTick() {}

// IDA_EA: 0x812F798
// IDA_NAME: _ZN12advancealtar14Timer_MainTick12dispatch_sigEiij
// DEMANGLED: advancealtar::Timer_MainTick::dispatch_sig(int,int,unsigned int)
int Timer_MainTick::dispatch_sig(int, int, unsigned int) {
    G_CGameManager()->onTimeAdvanceAltar();
    RegistNextTimerGage();
    return 1;
}

// IDA_EA: 0x812F7B8
// IDA_NAME: _ZN12advancealtar14Timer_MainTick19RegistNextTimerGageEv
// DEMANGLED: advancealtar::Timer_MainTick::RegistNextTimerGage(void)
int Timer_MainTick::RegistNextTimerGage() {
    return G_TimerQueue()->InsertTimerInMilisecond(8, 0, 166, 1000, 0, 0);
}

// IDA_EA: 0x863D12E
// IDA_NAME: _ZN12advancealtar18Timer_StageControlC2Ev
Timer_StageControl::Timer_StageControl() : TimerSig() {}

// IDA_EA: 0x813DB96 / 0x813DBC6
// IDA_NAME: _ZN12advancealtar18Timer_StageControlD2Ev / _ZN12advancealtar18Timer_StageControlD0Ev
Timer_StageControl::~Timer_StageControl() {}

// IDA_EA: 0x812F806
// IDA_NAME: _ZN12advancealtar18Timer_StageControl12dispatch_sigEiij
// DEMANGLED: advancealtar::Timer_StageControl::dispatch_sig(int,int,unsigned int)
int Timer_StageControl::dispatch_sig(int stage_index,
                                     int timer_message,
                                     unsigned int end_type) {
    StageControl* const stage =
        G_CGameManager()->getAdvanceAltar(stage_index);
    if (!stage) {
        return 0;
    }
    CUser* const owner = stage->charac_data.getUser();
    if (stage->getIndex() != stage_index || !owner ||
        owner->getAdvanceAltarIndex() != stage_index ||
        owner->getAdvanceAltar() != stage) {
        LogManager::logFormat(
            1,
            "advance_altar_lifecycle.cpp",
            "Timer_StageControl::dispatch_sig",
            0,
            "[ADVANCE_ALTAR][STALE_TIMER] index=%d message=%d stage_index=%d owner=%d owner_index=%d",
            stage_index,
            timer_message,
            stage->getIndex(),
            owner ? owner->getCurCharacNo() : 0,
            owner ? owner->getAdvanceAltarIndex() : -1);
        return 0;
    }
    const int stage_state = static_cast<int>(stage->getProcStage()->getState());
    const bool valid_state =
        timer_message == 168 ? stage_state == 5 :
        (timer_message == 169 || timer_message == 170)
            ? stage_state == 6
            : true;
    if (!valid_state) {
        LogManager::logFormat(
            1,
            "advance_altar_lifecycle.cpp",
            "Timer_StageControl::dispatch_sig",
            0,
            "[ADVANCE_ALTAR][STALE_TIMER_STATE] index=%d message=%d state=%d owner=%d",
            stage_index,
            timer_message,
            stage_state,
            owner->getCurCharacNo());
        return 0;
    }
    stage->onTimer(timer_message, static_cast<StageEndType::T>(end_type));
    return 1;
}

// IDA_EA: 0x812F850
// IDA_NAME: _ZN12advancealtar18Timer_StageControl21RegistTimerCheckFinshEi
// DEMANGLED: advancealtar::Timer_StageControl::RegistTimerCheckFinsh(int)
int Timer_StageControl::RegistTimerCheckFinsh(int stage_index) {
    return G_TimerQueue()->InsertTimerInMilisecond(
        8, stage_index, 167, 5000, 167, 0);
}

// IDA_EA: 0x812F89C
// IDA_NAME: _ZN12advancealtar18Timer_StageControl22RegistTimerClearRewardEiNS_12StageEndType1TE
// DEMANGLED: advancealtar::Timer_StageControl::RegistTimerClearReward(int,advancealtar::StageEndType::T)
int Timer_StageControl::RegistTimerClearReward(int stage_index,
                                               StageEndType::T end_type) {
    return G_TimerQueue()->InsertTimerInMilisecond(
        8, stage_index, 168, 3000, 168, static_cast<int>(end_type));
}

// IDA_EA: 0x812F8EC
// IDA_NAME: _ZN12advancealtar18Timer_StageControl15RegistTimerEPLPEiNS_12StageEndType1TE
// DEMANGLED: advancealtar::Timer_StageControl::RegistTimerEPLP(int,advancealtar::StageEndType::T)
int Timer_StageControl::RegistTimerEPLP(int stage_index,
                                        StageEndType::T end_type) {
    return G_TimerQueue()->InsertTimerInMilisecond(
        8, stage_index, 169, 1000, 169, static_cast<int>(end_type));
}

// IDA_EA: 0x812F956
// IDA_NAME: _ZN12advancealtar18Timer_StageControl28RegistTimerEPLPReturnVillageEi
// DEMANGLED: advancealtar::Timer_StageControl::RegistTimerEPLPReturnVillage(int)
int Timer_StageControl::RegistTimerEPLPReturnVillage(int stage_index) {
    return G_TimerQueue()->InsertTimerInMilisecond(
        8, stage_index, 170, 50000, 170, 0);
}

}  // namespace advancealtar
