#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

namespace {

// IDA_EA: 0x889234D
// IDA_NAME: _Z15getItemChecksumiiiii
// DEMANGLED: getItemChecksum(int,int,int,int,int)
//
// The original global helper is pure arithmetic.  Keep the exact operation
// local to this C-owned recovery so CInventory does not create an otherwise
// unrelated cross-owner unresolved edge.
int CalculateItemChecksum(int slot,
                          int item_id,
                          int item_count,
                          int upgrade,
                          int endurance) {
    const int checksum_input =
        10 * slot * (endurance + upgrade + item_id + item_count);
    const int quotient = checksum_input / 10;
    return static_cast<int>(static_cast<uint32_t>(quotient) >> 16) +
           static_cast<int>(static_cast<uint16_t>(quotient));
}

uint16_t GetChecksumEndurance(const Inven_Item& item) {
    return item.avatar_select_;
}

}  // namespace

// IDA_EA: 0x850AE7A
// IDA_NAME: _ZNK10CInventory12makeCheckSumEii
// DEMANGLED: CInventory::makeCheckSum(int,int)const
int CInventory::makeCheckSum(int pvp_win_point, int seed) const {
    int checksum = 0;

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const item = GetInvenRef(0, slot);
        const int item_id = item->getKey();
        if (item_id) {
            checksum += CalculateItemChecksum(
                slot,
                item_id,
                1,
                item->GetUpgrade(),
                GetChecksumEndurance(*item));
        }
    }

    for (int slot = 3;
         slot <= oldsrv::config::kMainInventoryLastSlot;
         ++slot) {
        const Inven_Item* const item = GetInvenRef(1, slot);
        const int item_id = item->getKey();
        if (item_id) {
            const int item_count = item->item_category_ == 1
                                       ? 1
                                       : item->get_add_info();
            checksum += CalculateItemChecksum(
                slot,
                item_id,
                item_count,
                item->GetUpgrade(),
                GetChecksumEndurance(*item));
        }
    }

    checksum += pvp_win_point + static_cast<int>(money_) +
                static_cast<int>(coin_) + static_cast<int>(pay_coin_) +
                static_cast<int>(event_coin_);
    return checksum ^ seed;
}
