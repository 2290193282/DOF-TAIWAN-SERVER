#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_FORCE_CHANGE_GROW_TYPE_QUEST_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_FORCE_CHANGE_GROW_TYPE_QUEST_RUNTIME_H_

class CUser;
class Quest;

// IDA_EA: 0x86686ED
// IDA_NAME: _Z40_postCheckForceChangeGrowType4AheadQuestP5CUserP5Quest
// DEMANGLED: _postCheckForceChangeGrowType4AheadQuest(CUser *,Quest *)
char _postCheckForceChangeGrowType4AheadQuest(CUser* user, Quest* quest);

// IDA_EA: 0x8668823
// IDA_NAME: _Z29_postCheckForceChangeGrowTypeP5CUserii
// DEMANGLED: _postCheckForceChangeGrowType(CUser *,int,int)
void _postCheckForceChangeGrowType(CUser* user,
                                   int first_grow_type,
                                   int second_grow_type);

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_FORCE_CHANGE_GROW_TYPE_QUEST_RUNTIME_H_
