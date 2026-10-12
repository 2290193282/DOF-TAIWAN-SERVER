#ifndef OLDSRV_OLD_DATA_PVF_ITEMSHOP_SECRET_SHOP_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ITEMSHOP_SECRET_SHOP_SCRIPT_H_

#include <stdint.h>
#include <stddef.h>

#include <map>
#include <set>
#include <utility>
#include <vector>


// IDA_EA: 0x8A832AA, 0x8A85BCA, 0x8A832EE, 0x8A832FE, 0x83720C8, 0x83720DE, 0x8A8354E, 0x8A83312, 0x83CE6D6, 0x8A83622, 0x8372020, 0x83720F4, 0x837D756, 0x8A82804, 0x8A828F8, 0x8A8293C, 0x8A82998, 0x8A82A0E, 0x8A82B34, 0x8A82C70, 0x8A82D44, 0x8A82F6A, 0x8A82FF6, 0x8A8306C, 0x8A830E2, 0x8A83188, 0x8A81CF0.
class CMTRand;

namespace secretshop {

// IDA unit: 07_npc_shop_dialog/secretshop_SALE_INFO, exposed here only as the
// narrow value carried by the B-owned SecretShop DataManager/PVF table.
struct SALE_INFO {
    SALE_INFO();
    ~SALE_INFO();

    bool operator==(int item_index);

    int32_t item_index;
    uint8_t buy_rule;
    int32_t price;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
    uint8_t event_item;
};

}  // namespace secretshop

struct stDungeonNpc {
    int32_t selector_index;
    int32_t npc_index;
    int32_t ratio;
};

struct stLevelSection {
    bool operator<(const stLevelSection& other) const;

    int32_t section_index;
    int32_t min_level;
    int32_t max_level;
};

struct stSaleInfo {
    int32_t item_index;
    uint8_t buy_rule;
    int32_t price;
    int32_t value_0c;
    int32_t value_10;
    int32_t ratio;
};

struct stDungeonSales {
    stDungeonSales();
    ~stDungeonSales();
    stDungeonSales(const stDungeonSales& other);

    int32_t selector_index;
    int32_t sale_count;
    std::map<int, stSaleInfo> sales;
};

struct stNpcSales {
    stNpcSales();
    ~stNpcSales();
    stNpcSales(const stNpcSales& other);

    int32_t npc_index;
    std::map<int, stDungeonSales> dungeon_sales;
    std::map<int, stDungeonSales> level_sales;
};

class SecretShopScript {
public:
    typedef std::multimap<int, stDungeonNpc> NpcMultiMap;
    typedef std::pair<NpcMultiMap::iterator, NpcMultiMap::iterator>
        NpcRange;

    SecretShopScript();
    ~SecretShopScript();

    void clear();
    void verifyNpc(NpcMultiMap& npcs);
    int GetNpcByDungeonIdx(int& npc_index, int random_value, int dungeon_index);
    int GetNpcByDungeonLev(int& npc_index, int random_value, int standard_level);
    int GetNpcIdx(int& npc_index, int random_value, NpcRange& range);
    int GetItemByDungeonIdx(CMTRand* random,
                            std::vector<secretshop::SALE_INFO>& items,
                            int npc_index,
                            int dungeon_index,
                            bool include_event);
    int GetItemByDungeonLev(CMTRand* random,
                            std::vector<secretshop::SALE_INFO>& items,
                            int npc_index,
                            int standard_level,
                            bool include_event);
    int GetItemByEvent(CMTRand* random,
                       std::vector<secretshop::SALE_INFO>& items);
    int GetItems(CMTRand* random,
                 std::vector<secretshop::SALE_INFO>& items,
                 stDungeonSales& sales);
    int IsExistItem(std::vector<secretshop::SALE_INFO>& items, int item_index);
    int GetDomainRate(stDungeonSales& sales);
    void CopyItem(secretshop::SALE_INFO& dest, stSaleInfo& src);
    int GetLevelIdx(int standard_level);
    int GetRandItemPrice(int price);

    NpcMultiMap dungeon_npcs;
    NpcMultiMap level_npcs;
    std::set<stLevelSection> level_sections;
    std::map<int, stNpcSales> npc_sales;
    stDungeonSales event_sales;
    int32_t price_random_percent;
    int32_t eplp_wait_time;
};

int importSecretShopScript(SecretShopScript* script, const char* filename);

static_assert(sizeof(secretshop::SALE_INFO) == 0x1Cu,
              "secretshop::SALE_INFO IDA ABI size must stay 0x1C");
static_assert(offsetof(secretshop::SALE_INFO, buy_rule) == 0x04u,
              "secretshop::SALE_INFO::buy_rule offset must stay 0x04");
static_assert(offsetof(secretshop::SALE_INFO, price) == 0x08u,
              "secretshop::SALE_INFO::price offset must stay 0x08");
static_assert(offsetof(secretshop::SALE_INFO, event_item) == 0x18u,
              "secretshop::SALE_INFO::event_item offset must stay 0x18");
static_assert(sizeof(stDungeonNpc) == 0x0Cu,
              "stDungeonNpc IDA ABI size must stay 0x0C");
static_assert(sizeof(stLevelSection) == 0x0Cu,
              "stLevelSection IDA ABI size must stay 0x0C");
static_assert(sizeof(stSaleInfo) == 0x18u,
              "stSaleInfo IDA ABI size must stay 0x18");
static_assert(offsetof(stSaleInfo, buy_rule) == 0x04u,
              "stSaleInfo::buy_rule offset must stay 0x04");
static_assert(offsetof(stSaleInfo, ratio) == 0x14u,
              "stSaleInfo::ratio offset must stay 0x14");
static_assert(sizeof(stDungeonSales) == 0x20u,
              "stDungeonSales IDA ABI size must stay 0x20");
static_assert(sizeof(stNpcSales) == 0x34u,
              "stNpcSales IDA ABI size must stay 0x34");
static_assert(sizeof(SecretShopScript) == 0x88u,
              "SecretShopScript IDA ABI size must stay 0x88");

#endif  // OLDSRV_OLD_DATA_PVF_ITEMSHOP_SECRET_SHOP_SCRIPT_H_
