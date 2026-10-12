#ifndef OLDSRV_OLD_DATA_PVF_EVENT_CONDITION_EVENT_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_CONDITION_EVENT_SCRIPT_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/event_value_types.h"


// IDA_EA: 0x890E868
// IDA_NAME: _Z26importConditionEventScriptPKcP20stConditionEventInfo
// DEMANGLED: importConditionEventScript(char const*,stConditionEventInfo *)
int importConditionEventScript(const char* filename,
                               stConditionEventInfo* info);

// IDA_EA: 0x8913891
// IDA_NAME: _Z36importConditionEventChkDungeonScriptPKcP24ConditionLevelChkDungeon
// DEMANGLED: importConditionEventChkDungeonScript(char const*,ConditionLevelChkDungeon *)
int importConditionEventChkDungeonScript(const char* filename,
                                         ConditionLevelChkDungeon* info);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_CONDITION_EVENT_SCRIPT_H_
