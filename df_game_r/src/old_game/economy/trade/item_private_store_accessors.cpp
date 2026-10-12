#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/npc/need_material_data_manager.h"

// IDA_EA: 0x822C84A
// IDA_NAME: _ZNK5CItem9get_priceEv
// DEMANGLED: CItem::get_price(void)const
int CItem::get_price() const {
    return price;
}

// IDA_EA: 0x822C856
// IDA_NAME: _ZNK5CItem12GetCashPriceEv
// DEMANGLED: CItem::GetCashPrice(void)const
int CItem::GetCashPrice() const {
    return cash_price;
}

// IDA_EA: 0x822C862
// IDA_NAME: _ZNK5CItem13GetMedalPriceEv
// DEMANGLED: CItem::GetMedalPrice(void)const
int CItem::GetMedalPrice() const {
    return medal_price;
}

// IDA_EA: 0x850D6F4
// IDA_NAME: _ZNK5CItem15GetNeedMaterialEv
// DEMANGLED: CItem::GetNeedMaterial(void)const
const std::pair<int, int>* CItem::GetNeedMaterial() const {
    const NeedMaterialDataManager::NeedMaterialPair* const override_value =
        NeedMaterialDataManager::getInstance()->getNeedMaterial(this);
    return override_value->first == -1 ? &script_value_pair_
                                       : override_value;
}
