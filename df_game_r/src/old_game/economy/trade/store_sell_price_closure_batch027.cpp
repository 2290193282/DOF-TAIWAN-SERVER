#include "old_game/economy/shop/npc_runtime.h"

#include "common/config.h"
#include "common/manager/find_price_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8618EA0
// IDA_NAME: _ZN5Store16GetSellItemPriceER10Inven_ItemPK5CItemsbRi
// DEMANGLED: Store::GetSellItemPrice(Inven_Item &,CItem const*,short,bool,int &)
int Store::GetSellItemPrice(Inven_Item& item,
                            const CItem* item_data,
                            short count,
                            bool,
                            int& price) {
    const bool is_special_price_type =
        item_data->vtable_->is_random_option(item_data) != 0;

    if (item.isEquipableItemType()) {
        const unsigned int equip_type = static_cast<const CEquipItem*>(
                                            item_data)->item_type_;
        if (equip_type == oldsrv::config::kEquipmentTypeTitle ||
            equip_type <= oldsrv::config::kAvatarEquipmentTypeLast) {
            return 23;
        }
        const CEquipItem* const equipment =
            reinterpret_cast<const CEquipItem*>(item_data);
        const int maximum_endurance = equipment->get_endurance();
        uint16_t endurance = item.avatar_select_;
        if (endurance > maximum_endurance) {
            endurance = static_cast<unsigned short>(maximum_endurance);
            item.avatar_select_ = endurance;
        }
        price = getItemSellPrice(item_data->GetSellPrice(), endurance,
                                 maximum_endurance,
                                 is_special_price_type);
        return 0;
    }

    if (item.get_add_info() < count || count <= 0) return 17;
    price = getItemSellPrice(item_data->GetSellPrice(), 0, 0,
                             is_special_price_type) *
            count;
    return 0;
}
