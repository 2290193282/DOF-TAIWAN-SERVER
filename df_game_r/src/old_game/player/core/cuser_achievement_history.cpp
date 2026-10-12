#include "old_game/player/core/cuser.h"

// IDA_EA: 0x868512A
// IDA_NAME: _ZN15cUserHistoryLog19AchievementCompleteEi
// DEMANGLED: cUserHistoryLog::AchievementComplete(int)
int cUserHistoryLog::AchievementComplete(int achievement_id) {
    return owner_->LogHistory("AchieveComplete,%d", achievement_id);
}

// IDA_EA: 0x868514E
// IDA_NAME: _ZN15cUserHistoryLog13QuestCompleteEi
// DEMANGLED: cUserHistoryLog::QuestComplete(int)
int cUserHistoryLog::QuestComplete(int quest_index) {
    return owner_->LogHistory("QuestComplete,%d", quest_index);
}
