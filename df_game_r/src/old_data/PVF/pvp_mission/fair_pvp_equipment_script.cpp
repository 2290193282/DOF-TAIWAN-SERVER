#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <stdint.h>

#include <string>


// IDA_EA: 0x88945AD
// IDA_NAME: _Z30makeFairPVPEquipmentPackageKeyii
// DEMANGLED: makeFairPVPEquipmentPackageKey(int,int)
int makeFairPVPEquipmentPackageKey(int character_job, int rank) {
    const uint32_t low = static_cast<uint16_t>(character_job);
    const uint32_t high = static_cast<uint16_t>(rank);
    return static_cast<int>(low | (high << 16));
}

// IDA_EA: 0x88945C6
// IDA_NAME: _Z40decreaseRankOfFairPVPEquipmentPackageKeyi
// DEMANGLED: decreaseRankOfFairPVPEquipmentPackageKey(int)
int decreaseRankOfFairPVPEquipmentPackageKey(int key) {
    return key - 0x10000;
}

// IDA_EA: 0x88FFBFC
// IDA_NAME: _Z22importFairPVPEquipmentPcPSt3mapIiSt6vectorIS1_I22STStackablePackageDataSaIS2_EESaIS4_EESt4lessIiESaISt4pairIKiS6_EEE
// DEMANGLED: importFairPVPEquipment(char *,FairPvpEquipmentMap *)
int importFairPVPEquipment(char* filename, FairPvpEquipmentMap* package_map) {
    if (!package_map) {
        return 0;
    }

    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string job_name;
    while (ScanType(tag, true) == 1) {
        if (tag == "[group]") {
            if (ScanStr(&job_name) != 1) {
                return 0;
            }

            const int character_job = ::getCharacterJob(job_name.c_str());
            bool success = false;
            const int rank = ScanInt(static_cast<bool*>(0));
            const int package_key =
                makeFairPVPEquipmentPackageKey(character_job, rank);

            FairPvpEquipmentPackageList package_list;
            ScanType(tag, true);
            while (tag == "[package selection]") {
                FairPvpEquipmentPackage package;
                while (1) {
                    const int item_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    STStackablePackageData package_data;
                    package_data.item_index = item_index;
                    package_data.item_count = ScanInt(static_cast<bool*>(0));
                    package.push_back(package_data);
                }

                package_list.push_back(package);
                ScanType(tag, true);
            }

            (*package_map)[package_key] = package_list;
        } else if (tag == "[/fair pvp package]") {
            break;
        }
    }

    return 1;
}

// IDA_EA: 0x88FFE75
// IDA_NAME: _Z29importUniqueRewardForAllUsersPcPSt3mapIiSt6vectorIS1_I22STStackablePackageDataSaIS2_EESaIS4_EESt4lessIiESaISt4pairIKiS6_EEE
// DEMANGLED: importUniqueRewardForAllUsers(char *,FairPvpEquipmentMap *)
int importUniqueRewardForAllUsers(char* filename,
                                  FairPvpEquipmentMap* package_map) {
    if (!package_map) {
        return 0;
    }

    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string job_name;
    while (ScanType(tag, true) == 1) {
        if (tag == "[group]") {
            if (ScanStr(&job_name) != 1) {
                return 0;
            }

            const int character_job = ::getCharacterJob(job_name.c_str());
            FairPvpEquipmentPackageList package_list;
            ScanType(tag, true);
            while (tag == "[package selection]") {
                bool success = false;
                FairPvpEquipmentPackage package;
                while (1) {
                    const int item_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    STStackablePackageData package_data;
                    package_data.item_index = item_index;
                    package_data.item_count = ScanInt(static_cast<bool*>(0));
                    package.push_back(package_data);
                }

                package_list.push_back(package);
                ScanType(tag, true);
            }

            (*package_map)[character_job] = package_list;
        } else if (tag == "[/package]") {
            break;
        }
    }

    return 1;
}
