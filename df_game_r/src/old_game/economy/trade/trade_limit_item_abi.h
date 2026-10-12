#ifndef OLDSRV_GLOBAL_TRADE_LIMIT_ITEM_ABI_H_
#define OLDSRV_GLOBAL_TRADE_LIMIT_ITEM_ABI_H_

class CUser;
struct Inven_Item;

// IDA_EA: 0x812013C
// IDA_NAME: _Z23DecreaseTradeLimitCountR10Inven_Item
int DecreaseTradeLimitCount(Inven_Item& item);

// IDA_EA: 0x81201DC
// IDA_NAME: _Z26IsTradeLimitAttachTypeItemRK10Inven_Item
bool IsTradeLimitAttachTypeItem(const Inven_Item& item);

// IDA_EA: 0x8120242
// IDA_NAME: _Z32IsTradeLimitAttachTypeItemByUserR5CUserii
bool IsTradeLimitAttachTypeItemByUser(CUser& user,
                                      int inventory_type,
                                      int slot);

// IDA_EA: 0x81202A9
// IDA_NAME: _Z30CheckTradeLimitItemCountToMoveRK10Inven_ItemRb
bool CheckTradeLimitItemCountToMove(const Inven_Item& item, bool& can_move);

// IDA_EA: 0x812032E
// IDA_NAME: _Z36CheckItemTradeLimitCountToMoveByUserR5CUseriiRb
bool CheckItemTradeLimitCountToMoveByUser(CUser& user,
                                          int inventory_type,
                                          int slot,
                                          bool& can_move);

#endif  // OLDSRV_GLOBAL_TRADE_LIMIT_ITEM_ABI_H_
