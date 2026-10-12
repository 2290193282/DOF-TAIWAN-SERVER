#ifndef OLDSRV_OLD_DATA_PVF_DATA_PREMIUM_ITEM_DATA_H_
#define OLDSRV_OLD_DATA_PVF_DATA_PREMIUM_ITEM_DATA_H_

#include <stdint.h>

namespace premium_script_parsing {

// IDA unit: 12_pvf_datamanager_scripts/premium_script_parsing_STPremiumItemData.
struct STPremiumItemData {
    STPremiumItemData();
    ~STPremiumItemData();

    uint8_t flag_00;
    uint8_t flag_01;
    uint8_t padding_02[2];
    int32_t item_id;
};

}  // namespace premium_script_parsing
#endif  // OLDSRV_OLD_DATA_PVF_DATA_PREMIUM_ITEM_DATA_H_
