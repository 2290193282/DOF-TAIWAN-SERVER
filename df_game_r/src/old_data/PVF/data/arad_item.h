#ifndef OLDSRV_OLD_DATA_PVF_DATA_ARAD_ITEM_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ARAD_ITEM_H_

#include "old_game/economy/inventory/inventory.h"

namespace ARAD {

class ARAD_ITEM {
public:
    // IDA_EA: 0x8197375
    // IDA_NAME: _ZN4ARAD9ARAD_ITEM14arad_make_itemEmiR10Inven_Item
    // DEMANGLED: ARAD::ARAD_ITEM::arad_make_item(unsigned long,int,Inven_Item &)
    static int arad_make_item(unsigned long item_index,
                              int item_count,
                              Inven_Item& item);
};

}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ARAD_ITEM_H_
