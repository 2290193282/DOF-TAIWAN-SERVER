#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_EQUIPMENT_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_EQUIPMENT_SCRIPT_H_

#include "common/types/stackable_value_types.h"

#include <map>
#include <vector>


typedef std::vector<STStackablePackageData> FairPvpEquipmentPackage;
typedef std::vector<FairPvpEquipmentPackage> FairPvpEquipmentPackageList;
typedef std::map<int, FairPvpEquipmentPackageList> FairPvpEquipmentMap;

// IDA_EA: 0x88945AD
// IDA_NAME: _Z30makeFairPVPEquipmentPackageKeyii
// DEMANGLED: makeFairPVPEquipmentPackageKey(int,int)
int makeFairPVPEquipmentPackageKey(int character_job, int rank);

// IDA_EA: 0x88945C6
// IDA_NAME: _Z40decreaseRankOfFairPVPEquipmentPackageKeyi
// DEMANGLED: decreaseRankOfFairPVPEquipmentPackageKey(int)
int decreaseRankOfFairPVPEquipmentPackageKey(int key);

// IDA_EA: 0x88FFBFC
int importFairPVPEquipment(char* filename, FairPvpEquipmentMap* package_map);

// IDA_EA: 0x88FFE75
int importUniqueRewardForAllUsers(char* filename,
                                  FairPvpEquipmentMap* package_map);

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_EQUIPMENT_SCRIPT_H_
