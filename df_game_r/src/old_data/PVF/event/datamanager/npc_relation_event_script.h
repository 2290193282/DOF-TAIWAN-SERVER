#ifndef OLDSRV_DATAMANAGER_NPC_RELATION_EVENT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_NPC_RELATION_EVENT_SCRIPT_H_


class CNPCRelationEventManager;

// IDA_EA: 0x890D44F
// IDA_NAME: _Z28importNPCRelationEventScriptPKcP24CNPCRelationEventManager
// DEMANGLED: importNPCRelationEventScript(char const*,CNPCRelationEventManager *)
int importNPCRelationEventScript(const char* filename,
                                 CNPCRelationEventManager* manager);

#endif  // OLDSRV_DATAMANAGER_NPC_RELATION_EVENT_SCRIPT_H_
