#ifndef OLDSRV_OLD_GAME_INVENTORY_INTEGRATED_PVP_ITEM_ATTR_H_
#define OLDSRV_OLD_GAME_INVENTORY_INTEGRATED_PVP_ITEM_ATTR_H_

struct Inven_Item;

// IDA_EA: 0x84FC541
// IDA_NAME: _Z25_getIntegratedPvPItemAttrRK10Inven_Item
// DEMANGLED: _getIntegratedPvPItemAttr(Inven_Item const&)
int _getIntegratedPvPItemAttr(const Inven_Item& item);

// IDA_EA: 0x84FC5FF
// IDA_NAME: _Z24GetIntegratedPvPItemAttrRK10Inven_Item
// DEMANGLED: GetIntegratedPvPItemAttr(Inven_Item const&)
int GetIntegratedPvPItemAttr(const Inven_Item& item);

#endif  // OLDSRV_OLD_GAME_INVENTORY_INTEGRATED_PVP_ITEM_ATTR_H_
