#ifndef OLDSRV_GLOBAL_ITEM_LIST_H_
#define OLDSRV_GLOBAL_ITEM_LIST_H_

#include <stddef.h>
#include <stdint.h>

#include <ext/hash_map>
#include <map>
#include <vector>

#include "old_game/economy/shop/npc_dialog_value_types.h"

struct Inven_Item;


class CItem;
class CCeraShopGoods;
struct stAmplifyOption_t;

typedef __gnu_cxx::hash_map<int, CItem*> CItemGradeList;
typedef __gnu_cxx::hash_map<int, CItem*> CItemIndexList;
typedef __gnu_cxx::hash_map<int, CCeraShopGoods*> CItemGoodsList;
typedef __gnu_cxx::hash_map<unsigned int, int> CItemRestrictCodeList;

// IDA unit: 99_misc_global/CItemList.
// The original vector element stride is 0x14.  GetProperBonusItem
// @0x8512E06 proves the two signed interval bounds at +0x00/+0x04 and treats
// the upper bound as exclusive.  No executable cross-reference reads or
// writes the remaining three dwords, so their semantics remain explicitly
// unresolved instead of being guessed from a newer server.
struct BonusItem {
    int32_t minimum_value;      // +0x00, inclusive
    int32_t maximum_value;      // +0x04, exclusive
    int32_t unresolved_08;      // +0x08, no 0627 executable use found
    int32_t unresolved_0c;      // +0x0c, no 0627 executable use found
    int32_t unresolved_10;      // +0x10, no 0627 executable use found
};

static_assert(offsetof(BonusItem, minimum_value) == 0x00u,
              "BonusItem minimum bound must stay at +0x00");
static_assert(offsetof(BonusItem, maximum_value) == 0x04u,
              "BonusItem maximum bound must stay at +0x04");
static_assert(offsetof(BonusItem, unresolved_08) == 0x08u,
              "BonusItem third dword must stay at +0x08");
static_assert(offsetof(BonusItem, unresolved_0c) == 0x0cu,
              "BonusItem fourth dword must stay at +0x0c");
static_assert(offsetof(BonusItem, unresolved_10) == 0x10u,
              "BonusItem fifth dword must stay at +0x10");
static_assert(sizeof(BonusItem) == 0x14u,
              "BonusItem IDA ABI size must stay 0x14");

enum ENUM_ITEM_CREATE_TYPE {
    ENUM_ITEM_CREATE_TYPE_DEFAULT = 0,
};

// IDA unit: 99_misc_global/CItemList. The active public baseline exposes the
// IDA_EA: GetRestrictCode(int) @0x817A10C,
// find_item(int) @0x8511E8A, find_item(char const*) @0x8511F1E,
// create_item(ENUM_ITEM_CREATE_TYPE, Inven_Item&, int) @0x8511FB8, and
// FindGoods(int) @0x8512358.
// CItemList's PVF loader restores the original parse/sync/materialize/register
// sequence in the B-owned data tree.
class CItemList {
public:
    // IDA_EA: 0x817A10C
    int GetRestrictCode(int item_id);
    CItem* find_item(int item_index);
    CItem* find_item(const char* item_name);
    // IDA_EA: 0x8511738
    // IDA_NAME: _ZN9CItemList15load_item_filesEv
    // DEMANGLED: CItemList::load_item_files(void)
    int load_item_files();
    // IDA_EA: 0x85123DA
    // IDA_NAME: _ZN9CItemList15Load_Goods_FileEPKc
    // DEMANGLED: CItemList::Load_Goods_File(char const*)
    bool Load_Goods_File(const char* filename);
    // IDA_EA: 0x8510FFA
    // IDA_NAME: _ZN9CItemList4initEv
    // DEMANGLED: CItemList::init(void)
    int init();
    // IDA_EA: 0x8510F06
    // IDA_NAME: _ZN9CItemList7destroyEv
    // DEMANGLED: CItemList::destroy(void)
    CItemList* destroy();
    // IDA_EA: 0x851129E
    int insert_item(int grade, int item_index, CItem* item);
    // IDA_EA: 0x869476E
    int GetQuestItemDropBouns(const unsigned long& item_drop_bonus) const;
    void create_item(ENUM_ITEM_CREATE_TYPE create_type,
                     Inven_Item& item,
                     int add_info);
    // IDA_EA: 0x8512230
    // IDA_NAME: _ZN9CItemList14get_lotto_itemEiPc
    // DEMANGLED: CItemList::get_lotto_item(int,char *)
    // The old ABI deliberately passes the canonical packed Inven_Item as a
    // char pointer.  Keep that signature so existing lottery callers retain
    // the original symbol and return-value convention.
    Inven_Item* get_lotto_item(int item_id, char* output);
    // IDA_EA: 0x8512358
    CCeraShopGoods* FindGoods(int goods_no);
    // IDA_EA: 0x8512930
    // IDA_NAME: _ZNK9CItemList20getCeraShopBonusItemEiRmRj
    // DEMANGLED: CItemList::getCeraShopBonusItem(
    //     int,unsigned long &,unsigned int &)const
    unsigned int* getCeraShopBonusItem(int used_cera,
                                       unsigned long& item_id,
                                       unsigned int& count) const;
    // IDA_EA: 0x8539E38.  This points to the 201 grade buckets allocated by
    // CItemList::init and is the source relation used by ItemGenRateTable.
    const CItemGradeList* getItemGradeList() const;

    // IDA_EA: 0x8512E06
    // IDA_NAME: _ZN9CItemList18GetProperBonusItemEj
    // DEMANGLED: CItemList::GetProperBonusItem(unsigned int)
    BonusItem* GetProperBonusItem(unsigned int value);

    // IDA_EA: 0x854C596
    // IDA_NAME: _ZNK9CItemList19GetVendingBonusRateEv
    // DEMANGLED: CItemList::GetVendingBonusRate(void) const
    int GetVendingBonusRate() const;

private:
    friend class CDataManager;
    friend struct CItemListAbiVerifier;

    CItemGradeList* item_grade_list_;  // +0x00, 201 grade buckets
    CItemIndexList items_by_index_;    // +0x04
    CItemGoodsList goods_by_index_;    // +0x18
    CItemRestrictCodeList restrict_codes_by_item_;  // +0x2c
    std::vector<stCeraShopBonusInfo_t> cera_shop_bonus_items_;  // +0x40
    int32_t vending_bonus_rate_;  // +0x4c
    std::vector<BonusItem> vending_bonus_items_;  // +0x50
    std::map<unsigned long, int> quest_drop_bonus_items_;  // +0x5c

    CItemList();
    ~CItemList();
    CItemList(const CItemList&);
    CItemList& operator=(const CItemList&);
};

// Keep the access-control boundary intact while making every physical field
// independently fail the i386 build if libstdc++ or a declaration drifts.
struct CItemListAbiVerifier {
    static_assert(offsetof(CItemList, item_grade_list_) == 0x00u,
                  "CItemList grade buckets must stay at +0x00");
    static_assert(offsetof(CItemList, items_by_index_) == 0x04u,
                  "CItemList item index must stay at +0x04");
    static_assert(offsetof(CItemList, goods_by_index_) == 0x18u,
                  "CItemList goods index must stay at +0x18");
    static_assert(offsetof(CItemList, restrict_codes_by_item_) == 0x2cu,
                  "CItemList restriction index must stay at +0x2c");
    static_assert(offsetof(CItemList, cera_shop_bonus_items_) == 0x40u,
                  "CItemList Cera bonus vector must stay at +0x40");
    static_assert(offsetof(CItemList, vending_bonus_rate_) == 0x4cu,
                  "CItemList vending bonus rate must stay at +0x4c");
    static_assert(offsetof(CItemList, vending_bonus_items_) == 0x50u,
                  "CItemList vending bonus vector must stay at +0x50");
    static_assert(offsetof(CItemList, quest_drop_bonus_items_) == 0x5cu,
                  "CItemList quest-drop map must stay at +0x5c");
};

static_assert(sizeof(CItemList) == 0x74u,
              "CItemList IDA ABI size must stay 0x74");


#endif  // OLDSRV_GLOBAL_ITEM_LIST_H_
