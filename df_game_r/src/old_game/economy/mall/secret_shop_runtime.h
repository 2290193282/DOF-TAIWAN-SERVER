#ifndef OLDSRV_OLD_GAME_NPC_SECRET_SHOP_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_SECRET_SHOP_RUNTIME_H_

#include <stdint.h>
#include <map>
#include <vector>

#include "old_data/PVF/itemshop/secret_shop_script.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"

class CMTRand;

class CUser;
struct Inven_Item;

enum eBuyRule {
    eBuyRule_Gold = 0,
    eBuyRule_Recipe = 1,
};

namespace secretshop {

class IBuyRule;
struct SECRET_SHOP_INFO;

// IDA unit: 07_npc_shop_dialog/secretshop_CSecretShopStatistic.
class CSecretShopStatistic {
public:
    // IDA_EA: 0x85FBEB8
    // IDA_NAME: _ZN10secretshop20CSecretShopStatisticC2Ev
    // DEMANGLED: secretshop::CSecretShopStatistic::CSecretShopStatistic(void)
    CSecretShopStatistic();

    // IDA_EA: 0x85FBF2C
    // IDA_NAME: _ZN10secretshop20CSecretShopStatisticD2Ev
    // DEMANGLED: secretshop::CSecretShopStatistic::~CSecretShopStatistic()
    ~CSecretShopStatistic();

    // IDA_EA: 0x85FCFEC
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic5ClearEv
    // DEMANGLED: secretshop::CSecretShopStatistic::Clear(void)
    void Clear();

    // IDA_EA: 0x85FD024
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic9GetNpcPosEi
    // DEMANGLED: secretshop::CSecretShopStatistic::GetNpcPos(int)
    int GetNpcPos(int npc_id);

    // IDA_EA: 0x85FD05E
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic11GetNpcIndexEi
    // DEMANGLED: secretshop::CSecretShopStatistic::GetNpcIndex(int)
    int GetNpcIndex(int npc_pos);

    // IDA_EA: 0x85FD090
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic10RecordShowEiii
    // DEMANGLED: secretshop::CSecretShopStatistic::RecordShow(int,int,int)
    void RecordShow(int npc_id, int dungeon_index, int member_count);

    // IDA_EA: 0x85FD0D4
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic9RecordBuyEii
    // DEMANGLED: secretshop::CSecretShopStatistic::RecordBuy(int,int)
    void RecordBuy(int npc_id, int dungeon_index);

    // IDA_EA: 0x85FD108
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic11RecordPriceEiii
    // DEMANGLED: secretshop::CSecretShopStatistic::RecordPrice(int,int,int)
    void RecordPrice(int npc_id, int dungeon_index, int price);

    // IDA_EA: 0x85FD13E
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic14GetDungeonDataEii
    // DEMANGLED: secretshop::CSecretShopStatistic::GetDungeonData(int,int)
    SECRET_SHOP_STATISTIC_DATA* GetDungeonData(int npc_id, int dungeon_index);

    // IDA_EA: 0x85FD278
    // IDA_NAME: _ZN10secretshop20CSecretShopStatistic23SendSecretShopStatisticEv
    // DEMANGLED: secretshop::CSecretShopStatistic::SendSecretShopStatistic(void)
    bool SendSecretShopStatistic();

private:
    std::map<int, SECRET_SHOP_STATISTIC_DATA> dungeon_data_[3];
};

// IDA unit: 07_npc_shop_dialog/secretshop_IBuyRule.
class IBuyRule {
public:
    virtual int BuyItem(CUser* user,
                        SECRET_SHOP_INFO& user_shop,
                        int item_id,
                        int count) = 0;

    // IDA_EA: 0x85FB55E
    // IDA_NAME: _ZN10secretshop8IBuyRule21SendSecretShopBuyItemEP5CUseriR10Inven_Itemiii
    // DEMANGLED: secretshop::IBuyRule::SendSecretShopBuyItem(CUser *,int,Inven_Item &,int,int,int)
    void SendSecretShopBuyItem(CUser* user,
                               int inventory_slot,
                               Inven_Item& item,
                               int material_item_id,
                               int material_count,
                               int remaining_count);

    // IDA_EA: 0x85FB6C8
    // IDA_NAME: _ZN10secretshop8IBuyRule23InsertItemIntoInventoryEP5CUseriiR10Inven_Item
    // DEMANGLED: secretshop::IBuyRule::InsertItemIntoInventory(CUser *,int,int,Inven_Item &)
    int InsertItemIntoInventory(CUser* user,
                                int item_id,
                                int count,
                                Inven_Item& item);

    // IDA_EA: 0x85FB7E4
    // IDA_NAME: _ZN10secretshop8IBuyRule10CheckLimitERNS_16SECRET_SHOP_INFOEiiRi
    // DEMANGLED: secretshop::IBuyRule::CheckLimit(secretshop::SECRET_SHOP_INFO &,int,int,int &)
    int CheckLimit(SECRET_SHOP_INFO& user_shop,
                   int item_id,
                   int count,
                   int& remaining_count);

    // IDA_EA: 0x85FB886
    // IDA_NAME: _ZN10secretshop8IBuyRule17LogValueStatisticEP5CUserj
    // DEMANGLED: secretshop::IBuyRule::LogValueStatistic(CUser *,unsigned int)
    int LogValueStatistic(CUser* user, unsigned int value);

    // IDA_EA: 0x85FB8B2
    // IDA_NAME: _ZN10secretshop8IBuyRule16LogCubeStatisticEP5CUserRSt4pairIiiE
    // DEMANGLED: secretshop::IBuyRule::LogCubeStatistic(CUser *,std::pair<int,int> &)
    void LogCubeStatistic(CUser* user, std::pair<int, int>& item_and_count);

    // IDA_EA: 0x85FBFC0
    // IDA_NAME: _ZN10secretshop8IBuyRuleC2ERNS_20CSecretShopStatisticE
    // DEMANGLED: secretshop::IBuyRule::IBuyRule(secretshop::CSecretShopStatistic &)
    explicit IBuyRule(CSecretShopStatistic& statistic);

protected:
    CSecretShopStatistic* statistic_;
};

class CBuyItembyGold : public IBuyRule {
public:
    // IDA_EA: 0x85FB8F0
    // IDA_NAME: _ZN10secretshop14CBuyItembyGold7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
    // DEMANGLED: secretshop::CBuyItembyGold::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
    virtual int BuyItem(CUser* user,
                        SECRET_SHOP_INFO& user_shop,
                        int item_id,
                        int count);

    // IDA_EA: 0x85FBFD8
    // IDA_NAME: _ZN10secretshop14CBuyItembyGoldC2ERNS_20CSecretShopStatisticE
    // DEMANGLED: secretshop::CBuyItembyGold::CBuyItembyGold(secretshop::CSecretShopStatistic &)
    explicit CBuyItembyGold(CSecretShopStatistic& statistic);
};

class CBuyItembyRecipe : public IBuyRule {
public:
    // IDA_EA: 0x85FBB62
    // IDA_NAME: _ZN10secretshop16CBuyItembyRecipe7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
    // DEMANGLED: secretshop::CBuyItembyRecipe::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
    virtual int BuyItem(CUser* user,
                        SECRET_SHOP_INFO& user_shop,
                        int item_id,
                        int count);

    // IDA_EA: 0x85FBFFC
    // IDA_NAME: _ZN10secretshop16CBuyItembyRecipeC2ERNS_20CSecretShopStatisticE
    // DEMANGLED: secretshop::CBuyItembyRecipe::CBuyItembyRecipe(secretshop::CSecretShopStatistic &)
    explicit CBuyItembyRecipe(CSecretShopStatistic& statistic);
};

// IDA unit: 07_npc_shop_dialog/secretshop_CSecretShop. The rule bodies are
// restored separately, but this is the only shared runtime layout.
class CSecretShop {
public:
    // IDA_EA: 0x85FA930
    // IDA_NAME: _ZN10secretshop11CSecretShopC2Ev
    // DEMANGLED: secretshop::CSecretShop::CSecretShop(void)
    CSecretShop();
    // IDA_EA: 0x85FAAC4
    // IDA_NAME: _ZN10secretshop11CSecretShopD2Ev
    // DEMANGLED: secretshop::CSecretShop::~CSecretShop()
    ~CSecretShop();
    // IDA_EA: 0x85FABAC
    // IDA_NAME: _ZN10secretshop11CSecretShop10LotteryNpcEiii
    // DEMANGLED: secretshop::CSecretShop::LotteryNpc(int,int,int)
    int LotteryNpc(int dungeon_index, int standard_level, int member_count);
    // IDA_EA: 0x85FAC68
    // IDA_NAME: _ZN10secretshop11CSecretShop12LotteryItemsERSt6vectorINS_9SALE_INFOESaIS2_EEiii
    // DEMANGLED: secretshop::CSecretShop::LotteryItems(std::vector<secretshop::SALE_INFO,std::allocator<secretshop::SALE_INFO>> &,int,int,int)
    int LotteryItems(std::vector<SALE_INFO>& items,
                     int npc_index,
                     int dungeon_index,
                     int standard_level);
    // IDA_EA: 0x85FAD20
    // IDA_NAME: _ZN10secretshop11CSecretShop12CheckLotteryEv
    // DEMANGLED: secretshop::CSecretShop::CheckLottery(void)
    int CheckLottery();
    // IDA_EA: 0x85FB16E
    // IDA_NAME: _ZN10secretshop11CSecretShop7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
    // DEMANGLED: secretshop::CSecretShop::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
    int BuyItem(CUser* user, SECRET_SHOP_INFO& user_shop, int item_id, int count);
    // IDA_EA: 0x85FB27C
    // IDA_NAME: _ZN10secretshop11CSecretShop7GetRuleE8eBuyRule
    // DEMANGLED: secretshop::CSecretShop::GetRule(eBuyRule)
    IBuyRule* GetRule(eBuyRule rule);
    // IDA_EA: 0x85FB2E0
    // IDA_NAME: _ZN10secretshop11CSecretShop17SendSecretShopNpcEP5CUseri
    // DEMANGLED: secretshop::CSecretShop::SendSecretShopNpc(CUser *,int)
    void SendSecretShopNpc(CUser* user, int npc_index);
    // IDA_EA: 0x85FB374
    // IDA_NAME: _ZN10secretshop11CSecretShop22SendSecretShopItemListEP5CUserRSt6vectorINS_9SALE_INFOESaIS4_EE
    // DEMANGLED: secretshop::CSecretShop::SendSecretShopItemList(CUser *,std::vector<secretshop::SALE_INFO,std::allocator<secretshop::SALE_INFO>> &)
    void SendSecretShopItemList(CUser* user, std::vector<SALE_INFO>& items);
    // IDA_EA: 0x85FB53A
    // IDA_NAME: _ZN10secretshop11CSecretShop23SendSecretShopStatisticEv
    // DEMANGLED: secretshop::CSecretShop::SendSecretShopStatistic(void)
    int SendSecretShopStatistic();

private:
    std::map<eBuyRule, IBuyRule*> rules_;
    // IDA C2/D2 and LotteryNpc use the live CMTRand pointer at +0x18.
    CMTRand* random_;
    CSecretShopStatistic statistic_;
};

// IDA unit: 07_npc_shop_dialog/secretshop_BUY_INFO.
struct BUY_INFO {
    // IDA_EA: 0x8AB1606
    // IDA_NAME: _ZN10secretshop8BUY_INFOC2Ev
    // DEMANGLED: secretshop::BUY_INFO::BUY_INFO(void)
    BUY_INFO();

    // IDA_EA: 0x8AB161E
    // IDA_NAME: _ZN10secretshop8BUY_INFOeqEi
    // DEMANGLED: secretshop::BUY_INFO::operator==(int)
    bool operator==(int item_id);

    // IDA_EA: 0x8AB1EFC
    // IDA_NAME: _ZN10secretshop8BUY_INFOD2Ev
    // DEMANGLED: secretshop::BUY_INFO::~BUY_INFO()
    ~BUY_INFO();

    int item_id;
    int count;
};

// IDA unit: 07_npc_shop_dialog/secretshop_RETAILER. Its base vector is the
// verified 0x0c ABI layout; adding a member would change it.
class RETAILER : public std::vector<SALE_INFO> {
public:
    // IDA_EA: 0x82A5346
    // IDA_NAME: _ZN10secretshop8RETAILERC2Ev
    // DEMANGLED: secretshop::RETAILER::RETAILER(void)
    RETAILER();

    // IDA_EA: 0x82A535A
    // IDA_NAME: _ZN10secretshop8RETAILERD2Ev
    // DEMANGLED: secretshop::RETAILER::~RETAILER()
    ~RETAILER();

    // IDA_EA: 0x8AB12A0
    // IDA_NAME: _ZN10secretshop8RETAILER11GetSaleInfoEi
    // DEMANGLED: secretshop::RETAILER::GetSaleInfo(int)
    SALE_INFO* GetSaleInfo(int item_id);

    // IDA_EA: 0x8AB15F2
    // IDA_NAME: _ZN10secretshop8RETAILER5clearEv
    // DEMANGLED: secretshop::RETAILER::clear(void)
    void clear();
};

// IDA unit: 07_npc_shop_dialog/secretshop_SHOPPER. Same 0x0c vector-only ABI.
class SHOPPER : public std::vector<BUY_INFO> {
public:
    // IDA_EA: 0x82A536E
    // IDA_NAME: _ZN10secretshop7SHOPPERC2Ev
    // DEMANGLED: secretshop::SHOPPER::SHOPPER(void)
    SHOPPER();

    // IDA_EA: 0x82A5382
    // IDA_NAME: _ZN10secretshop7SHOPPERD2Ev
    // DEMANGLED: secretshop::SHOPPER::~SHOPPER()
    ~SHOPPER();

    // IDA_EA: 0x8AB1344
    // IDA_NAME: _ZN10secretshop7SHOPPER10GetBuyInfoEi
    // DEMANGLED: secretshop::SHOPPER::GetBuyInfo(int)
    BUY_INFO* GetBuyInfo(int item_id);

    // IDA_EA: 0x8AB13E8
    // IDA_NAME: _ZN10secretshop7SHOPPER7BuyItemEii
    // DEMANGLED: secretshop::SHOPPER::BuyItem(int,int)
    void BuyItem(int item_id, int count);

    // IDA_EA: 0x8AB162E
    // IDA_NAME: _ZN10secretshop7SHOPPER5clearEv
    // DEMANGLED: secretshop::SHOPPER::clear(void)
    void clear();
};

// IDA unit: 07_npc_shop_dialog/secretshop_SECRET_SHOP_INFO.
struct SECRET_SHOP_INFO {
    // IDA_EA: 0x82A5396
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFOC2Ev
    // DEMANGLED: secretshop::SECRET_SHOP_INFO::SECRET_SHOP_INFO(void)
    SECRET_SHOP_INFO();

    // IDA_EA: 0x82A53DC
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFOD2Ev
    // DEMANGLED: secretshop::SECRET_SHOP_INFO::~SECRET_SHOP_INFO()
    ~SECRET_SHOP_INFO();

    // IDA_EA: 0x85FBE8A
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFO11GetSaleInfoEi
    // DEMANGLED: secretshop::SECRET_SHOP_INFO::GetSaleInfo(int)
    SALE_INFO* GetSaleInfo(int item_id);

    // IDA_EA: 0x8AB1642
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFO5clearEv
    // DEMANGLED: secretshop::SECRET_SHOP_INFO::clear(void)
    void clear();

    RETAILER retailer;
    SHOPPER shopper;
    uint8_t purchase_recorded;
    uint8_t padding_19[3];
};

// IDA unit: 07_npc_shop_dialog/secretshop_SECRET_SHOP_DATA.
struct SECRET_SHOP_DATA {
    // IDA_EA: 0x82A5422
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATAC2Ev
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::SECRET_SHOP_DATA(void)
    SECRET_SHOP_DATA();

    // IDA_EA: 0x82A549E
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATAD2Ev
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::~SECRET_SHOP_DATA()
    ~SECRET_SHOP_DATA();

    // IDA_EA: 0x8AB14B8
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA5clearEv
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::clear(void)
    bool clear();

    // IDA_EA: 0x8AB150C
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA6IsOpenEv
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::IsOpen(void)
    int IsOpen();

    // IDA_EA: 0x8AB1532
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA9SetBuyingEib
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::SetBuying(int,bool)
    unsigned int SetBuying(int user_pos, bool buying);

    // IDA_EA: 0x8AB155A
    // IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA13IsCompleteBuyEv
    // DEMANGLED: secretshop::SECRET_SHOP_DATA::IsCompleteBuy(void)
    int IsCompleteBuy();

    int state;
    SECRET_SHOP_INFO infos[4];
    uint8_t buying[4];
};

typedef char BUY_INFO_Size_Must_Be_0x08[
    (sizeof(BUY_INFO) == 0x08) ? 1 : -1];
typedef char RETAILER_Size_Must_Be_0x0c[
    (sizeof(RETAILER) == 0x0c) ? 1 : -1];
typedef char SHOPPER_Size_Must_Be_0x0c[
    (sizeof(SHOPPER) == 0x0c) ? 1 : -1];
typedef char SECRET_SHOP_INFO_Size_Must_Be_0x1c[
    (sizeof(SECRET_SHOP_INFO) == 0x1c) ? 1 : -1];
typedef char SECRET_SHOP_DATA_Size_Must_Be_0x78[
    (sizeof(SECRET_SHOP_DATA) == 0x78) ? 1 : -1];
typedef char CSecretShopStatistic_Size_Must_Be_0x48[
    (sizeof(CSecretShopStatistic) == 0x48) ? 1 : -1];
typedef char CSecretShop_Size_Must_Be_0x64[
    (sizeof(CSecretShop) == 0x64) ? 1 : -1];
typedef char IBuyRule_Size_Must_Be_0x08[
    (sizeof(IBuyRule) == 0x08) ? 1 : -1];
typedef char CBuyItembyGold_Size_Must_Be_0x08[
    (sizeof(CBuyItembyGold) == 0x08) ? 1 : -1];
typedef char CBuyItembyRecipe_Size_Must_Be_0x08[
    (sizeof(CBuyItembyRecipe) == 0x08) ? 1 : -1];

}  // namespace secretshop

#endif  // OLDSRV_OLD_GAME_NPC_SECRET_SHOP_RUNTIME_H_
