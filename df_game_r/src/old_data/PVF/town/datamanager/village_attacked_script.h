#ifndef OLDSRV_DATAMANAGER_VILLAGE_ATTACKED_SCRIPT_H_
#define OLDSRV_DATAMANAGER_VILLAGE_ATTACKED_SCRIPT_H_


struct stVillageAttacked_t;

// IDA_EA: importVillageAttackedScript @0x89093EE.
int importVillageAttackedScript(const char* filename,
                                stVillageAttacked_t& script);

#endif  // OLDSRV_DATAMANAGER_VILLAGE_ATTACKED_SCRIPT_H_
