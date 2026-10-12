#include "old_game/world/region/npc_relation_values.h"

#include <new>


typedef std::map<uint16_t, stNPCRelationEventInfo*> NPCRelationInfoMap;

typedef char stNPCFavorAndRewardInfo_Size_Must_Be_0x14[
    (sizeof(stNPCFavorAndRewardInfo) == 0x14) ? 1 : -1];
typedef char stNPCRelationEventInfo_Size_Must_Be_0x48[
    (sizeof(stNPCRelationEventInfo) == 0x48) ? 1 : -1];

// IDA_EA: 0x891C932
// IDA_NAME: _ZN23stNPCFavorAndRewardInfoC2Ev
// DEMANGLED: stNPCFavorAndRewardInfo::stNPCFavorAndRewardInfo(void)
stNPCFavorAndRewardInfo::stNPCFavorAndRewardInfo() {}

// IDA_EA: 0x891C8AA
// IDA_NAME: _ZN23stNPCFavorAndRewardInfoD2Ev
// DEMANGLED: stNPCFavorAndRewardInfo::~stNPCFavorAndRewardInfo()
stNPCFavorAndRewardInfo::~stNPCFavorAndRewardInfo() {}

// IDA_EA: 0x891C946
// IDA_NAME: _ZN22stNPCRelationEventInfoC2Ev
// DEMANGLED: stNPCRelationEventInfo::stNPCRelationEventInfo(void)
stNPCRelationEventInfo::stNPCRelationEventInfo() {}

// IDA_EA: 0x891C8BE
// IDA_NAME: _ZN22stNPCRelationEventInfoD2Ev
// DEMANGLED: stNPCRelationEventInfo::~stNPCRelationEventInfo()
stNPCRelationEventInfo::~stNPCRelationEventInfo() {}

// MIGRATED_TO: src/old_game/npc/npc_relation_event_manager.cpp
// The manager ctor/dtor are now the unique real CNPCRelationEventManager
// members.  This numbered source retains only relation-value implementations.

// Migration marker: the eight CNPCRelationEventManager Add/Get/Is bodies at
// 0x890CEEA..0x890D3D0 now have their sole provider in old_game/npc.
