#ifndef OLDSRV_DATAMANAGER_WORLD_DROP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_WORLD_DROP_SCRIPT_H_


struct WorldDropInfo;

// IDA_EA: 0x89022BE
// IDA_NAME: _Z15ImportWorldDropP13WorldDropInfoPKc
// DEMANGLED: ImportWorldDrop(WorldDropInfo *,char const*)
int ImportWorldDrop(WorldDropInfo* world_drop_info, const char* filename);

#endif  // OLDSRV_DATAMANAGER_WORLD_DROP_SCRIPT_H_
