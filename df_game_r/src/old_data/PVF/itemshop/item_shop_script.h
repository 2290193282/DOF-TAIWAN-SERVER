#ifndef OLDSRV_DATAMANAGER_ITEM_SHOP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_ITEM_SHOP_SCRIPT_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

// IDA unit: 07_npc_shop_dialog/ItemShopScript and CItemShop.
// These classes intentionally live in the old global namespace because the
// restored CDataManager and NPC shop call sites bind the old df_game_r symbols.
struct ItemShopScript {
    int script_index;                         // +0x00
    int shop_type;                            // +0x04
    int npc_index;                            // +0x08
    bool only_buy;                            // +0x0C
    char pad_0d[3];
    std::string message;                      // +0x10
    std::vector<int> sell_items;              // +0x14
    std::vector<std::string> tab_names;       // +0x20
    std::vector<int> sell_limit_items;        // +0x2C
    std::vector<std::vector<int> > one_a_day_item_lists;  // +0x38
    bool one_a_day_enabled;                   // +0x44
    char pad_45[3];
    int one_a_day_start_time;                 // +0x48
    int reserved_4c;                          // +0x4C

    ItemShopScript();
    ItemShopScript(const ItemShopScript& other);
    ~ItemShopScript();

    ItemShopScript* clear();
};

class CItemShop {
public:
    typedef std::map<int, ItemShopScript> ScriptMap;
    typedef std::vector<std::pair<int, int> > OneADaySelectionList;

    CItemShop();

    int loadItemShopFiles();
    int checkValidShopItem(unsigned int shop_id, unsigned long item_id);
    int updateOneADayItemList();
    int testupdateOneADayItemList();
    int setOneADayItemListNo(ItemShopScript& script);
    const OneADaySelectionList& GetOneADayItemList() const;
    int IsOneADayShop(int shop_id) const;
    int checkLimitNpcBuyItemList(unsigned int shop_id, unsigned long item_id);

private:
    ScriptMap scripts_;                       // +0x00
    OneADaySelectionList one_a_day_selected_; // +0x18
};

int initItemShopScript(char* src, char* filename);
int importItemShopScript(ItemShopScript* script, const char* filename);

#endif  // OLDSRV_DATAMANAGER_ITEM_SHOP_SCRIPT_H_
