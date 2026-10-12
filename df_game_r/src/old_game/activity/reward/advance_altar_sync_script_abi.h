#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_SYNC_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_SYNC_SCRIPT_ABI_H_

#include "old_game/activity/reward/advance_altar_shop_parameter.h"

#include <string>

namespace advancealtar {

// IDA unit: 12_pvf_datamanager_scripts/advancealtar::SyncScript.
// IDA_EA: truncateItemDescTable @0x8133E44, insertItemDescTable @0x8133EA6,
// getItemValueStirng @0x8134132.
// A-owned global sync ABI; it consumes the canonical B DB interface only.
class SyncScript {
public:
    static bool truncateItemDescTable();
    static bool insertItemDescTable();
    static bool getItemValueStirng(
        int ridable_id,
        const advancealtar::BuyShopData& shop_data,
        advancealtar::AdvanceAltarShopType::T shop_type,
        std::string& out);
};

}  // namespace advancealtar

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_SYNC_SCRIPT_ABI_H_
