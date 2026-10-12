#ifndef OLDSRV_DATAMANAGER_ETC_PARAMETER_SCRIPT_H_
#define OLDSRV_DATAMANAGER_ETC_PARAMETER_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "old_data/PVF/etc/etc_parameter_script.h"


// Legacy partial header: remaining B/C boundary functions have not yet been
// migrated.  Canonical typed ETC parsers are declared by the B header above.
// IDA_EA: 0x89113EC, 0x88FDB69, 0x8919127.
// IDA_EA: 0x890A427.
// IDA unit: 99_misc_global/Global etc-parameter RDAR import boundaries used
// by 12_pvf_datamanager_scripts/CDataManager::init.
struct PVPMAP_DATA;

struct SecurityReward {
    int32_t goblin_pad_item_id;
    int32_t goblin_pad_item_count;
    int32_t goblin_pad_period;
    int32_t goblin_pad_unknown;
    int32_t security_card_item_id;
    int32_t security_card_item_count;
    int32_t security_card_period;
    int32_t security_card_unknown;
    int32_t otp_item_id;
    int32_t otp_item_count;
    int32_t otp_period;
    int32_t otp_unknown;
};

int ImportSecurityReward(SecurityReward* reward);
int importGoldLimitByLevel(unsigned int* gold_limit_by_level);
int importQuestPieceScript(const char* filename, std::map<int, int>& table);
int loadPvpMapBasicDataScript(const char* filename,
                              std::vector<PVPMAP_DATA>* pvp_map_data)
    __asm__("_Z25loadPvpMapBasicDataScriptPKcPSt6vectorI11PVPMAP_DATASaIS2_EE");

#endif  // OLDSRV_DATAMANAGER_ETC_PARAMETER_SCRIPT_H_
