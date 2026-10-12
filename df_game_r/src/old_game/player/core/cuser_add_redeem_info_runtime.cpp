#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"

// IDA_EA: 0x86472C0
// IDA_NAME: _ZN5CUser14Add_RedeemInfoERK10Inven_Itemib
// DEMANGLED: CUser::Add_RedeemInfo(Inven_Item const&,int,bool)
int CUser::Add_RedeemInfo(const Inven_Item& item, int value, bool add_1000) {
    Redeem_Item::CRedeemItem* const redeem =
        static_cast<Redeem_Item::CRedeemItem*>(GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(6)));
    if (!redeem->isAddableFilter(item)) {
        return 0;
    }

    CItem* const item_data = G_CDataManager()->find_item(item.item_id_);
    history_log().RedeemItemAdd(
        item.item_id_, item_data->is_stackable() ? item.add_info_ : 1);
    return redeem->AddRedeemList(item, value, add_1000);
}
