#ifndef OLDSRV_DATAMANAGER_ARAD_ITEM_H_
#define OLDSRV_DATAMANAGER_ARAD_ITEM_H_

struct Inven_Item;
class CStackableItem;
class CUser;

namespace ARAD {

// The other ARAD_ITEM members are world/NPC state-machine behavior.  This
// data-manager entry point is the original item-prototype-to-mail-record
// conversion used by the event-reward pipeline.
class ARAD_ITEM {
public:
    // IDA_EA: 0x8197375
    // IDA_NAME: _ZN4ARAD9ARAD_ITEM14arad_make_itemEmiR10Inven_Item
    // DEMANGLED: ARAD::ARAD_ITEM::arad_make_item(unsigned long,int,Inven_Item &)
    static int arad_make_item(unsigned long item_index,
                              int item_count,
                              Inven_Item& item);

    // IDA_EA: 0x8196ED7
    // IDA_NAME: _ZN4ARAD9ARAD_ITEM27arad_secret_shop_event_itemEP5CUsermP14CStackableItem
    // DEMANGLED: ARAD::ARAD_ITEM::arad_secret_shop_event_item(CUser *,unsigned long,CStackableItem *)
    static void arad_secret_shop_event_item(CUser* user,
                                            unsigned long item_index,
                                            CStackableItem* item);
};

}  // namespace ARAD

#endif  // OLDSRV_DATAMANAGER_ARAD_ITEM_H_
