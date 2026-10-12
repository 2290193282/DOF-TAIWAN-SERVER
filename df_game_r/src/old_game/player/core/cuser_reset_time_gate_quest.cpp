#include "old_game/player/core/cuser_reset_time_gate_quest.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84F0878
// IDA_NAME: _ZN13CUserModifierC2Ev
// DEMANGLED: CUserModifier::CUserModifier(void)
CUserModifier::CUserModifier() {}

// IDA_EA: 0x84F0886
// IDA_NAME: _ZN23CUserResetTimeGateQuestC2Ev
// DEMANGLED: CUserResetTimeGateQuest::CUserResetTimeGateQuest(void)
CUserResetTimeGateQuest::CUserResetTimeGateQuest() : CUserModifier() {}

// IDA_EA: 0x84F07EC
// IDA_NAME: _ZN23CUserResetTimeGateQuest6ModifyEP5CUser
// DEMANGLED: CUserResetTimeGateQuest::Modify(CUser *)
int CUserResetTimeGateQuest::Modify(CUser* user) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR() ||
        !GlobalData::s_timeGate_) {
        return 0;
    }

    const int quest_index = GlobalData::s_timeGate_->getQuestIndex();
    UserQuest* const quest = user->getCurCharacQuestR();
    if (!quest || !quest->IsDoingQuest(quest_index)) {
        return 0;
    }

    UserQuest* const writable_quest = user->getCurCharacQuestW();
    return writable_quest ? writable_quest->giveup_quest(quest_index) : 0;
}
