#ifndef OLDSRV_OLD_GAME_NPC_NPC_DIALOG_VALUE_TYPES_H_
#define OLDSRV_OLD_GAME_NPC_NPC_DIALOG_VALUE_TYPES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x89E87EE in the companion source.


#include <stddef.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "old_game/activity/reward/event_base.h"

class CUser;
class CItem;
class Stream;
struct MSG_BUY_CERASHOP_ITEM;
struct Inven_Item;
class SpecialItemHandler;
class CCeraShopGoods;
struct STSpecailItem_Param;
struct STSpecailItem_Result;

// The old ELF names this parameter type in
// CCeraShop::_onBuySpecialItem.  The function does not inspect its value;
// preserving the fixed i386 underlying type preserves the original symbol ABI.
enum ENUM_CERA_SPECIALITEM : int;

// IDA unit: 99_misc_global/tagPackageItemSelection.  The package-selection
// vector in CCeraShop::stCeraShopItemParam_t stores this fixed 8-byte record.
struct tagPackageItemSelection {
    // IDA_EA: 0x817B0B6
    // IDA_NAME: _ZN23tagPackageItemSelectionD2Ev
    // DEMANGLED: tagPackageItemSelection::~tagPackageItemSelection()
    ~tagPackageItemSelection();

    uint32_t item_index;
    uint16_t package_index;
    uint16_t selection_index;
};

static_assert(sizeof(tagPackageItemSelection) == 0x08u,
              "tagPackageItemSelection IDA ABI size must stay 0x08");


// IDA unit: 07_npc_shop_dialog/AssignNpc.
struct AssignNpc {
    AssignNpc();
    ~AssignNpc();

    AssignNpc* clear();

    int npc_id;
    int dungeon_id;
    uint8_t flag;
    uint8_t pad_09[3];
    int area;
    int x;
    int y;
    int z;
};

// IDA unit: 07_npc_shop_dialog/NpcBuyLimitItem.
struct NpcBuyLimitItem {
    NpcBuyLimitItem();

    NpcBuyLimitItem* clear();

    int npc_id;
    int item_id;
    int buy_count;
};

// IDA unit: 07_npc_shop_dialog/stNPCRelationship_t.
struct stNPCRelationship_t {
    stNPCRelationship_t();

    uint16_t npc_id;
    uint16_t relationship;
    uint16_t mood;
    uint16_t reward_state;
    uint32_t update_time;
};

// IDA unit: 07_npc_shop_dialog/stQuestShoptInitCost.
struct stQuestShoptInitCost {
    stQuestShoptInitCost();

    int unknown_00;
    int unknown_04;
    int unknown_08;
};

// IDA unit: 07_npc_shop_dialog/stNPCEventItemInfo.
struct stNPCEventItemInfo {
    stNPCEventItemInfo& operator=(stNPCEventItemInfo rhs);
    ~stNPCEventItemInfo();

    int item_id;
    int count;
};

// IDA unit: 07_npc_shop_dialog/CCeraShopBonusItemEvent.
class CCeraShopBonusItemEvent : public CEventBase {
public:
    CCeraShopBonusItemEvent();
    virtual ~CCeraShopBonusItemEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int getBonusRate() const;

private:
    int bonus_rate;
};

// IDA unit: 07_npc_shop_dialog/CSecretShopEvent.
class CSecretShopEvent : public CEventBase {
public:
    CSecretShopEvent();
    virtual ~CSecretShopEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 07_npc_shop_dialog/STCeraShopItem.
//
// The old i386 object contains one libstdc++ string at +0x40 and has a
// physical vector stride of 0x88.  The named fields below are limited to
// values actually read by the original CeraShopGoods projection; the other
// fields deliberately retain raw names instead of inventing item-script
// semantics.
struct STCeraShopItem {
    // IDA_EA: 0x88FD974
    // IDA_NAME: _ZN14STCeraShopItemC2Ev
    // DEMANGLED: STCeraShopItem::STCeraShopItem(void)
    STCeraShopItem();

    // IDA_EA: 0x85263C4
    // IDA_NAME: _ZN14STCeraShopItemD2Ev
    // DEMANGLED: STCeraShopItem::~STCeraShopItem()
    ~STCeraShopItem();

    // IDA_EA: 0x88FD9C4
    // IDA_NAME: _ZN14STCeraShopItem5clearEv
    // DEMANGLED: STCeraShopItem::clear(void)
    STCeraShopItem* clear();

    // IDA_EA: 0x88FDB36
    // IDA_NAME: _ZN14STCeraShopItem15isLimitSaleItemEv
    // DEMANGLED: STCeraShopItem::isLimitSaleItem(void)
    bool isLimitSaleItem();

    // IDA_EA: 0x88FDB52
    // IDA_NAME: _ZN14STCeraShopItem12getCeraPriceEv
    // DEMANGLED: STCeraShopItem::getCeraPrice(void)
    int getCeraPrice();

    // IDA_EA: 0x88FDB5E
    // IDA_NAME: _ZN14STCeraShopItem12getGoldPriceEv
    // DEMANGLED: STCeraShopItem::getGoldPrice(void)
    int getGoldPrice();

    // IDA_EA: 0x891A89E
    // IDA_NAME: _ZN14STCeraShopItemeqERKS_
    // DEMANGLED: STCeraShopItem::operator==(STCeraShopItem const&)
    bool operator==(const STCeraShopItem& rhs);

    // IDA_EA: 0x8923E28
    // IDA_NAME: _ZN14STCeraShopItemC2ERKS_
    // DEMANGLED: STCeraShopItem::STCeraShopItem(STCeraShopItem const&)
    STCeraShopItem(const STCeraShopItem& rhs);

    // IDA_EA: 0x892408A
    // IDA_NAME: _ZN14STCeraShopItemaSERKS_
    // DEMANGLED: STCeraShopItem::operator=(STCeraShopItem const&)
    STCeraShopItem& operator=(const STCeraShopItem& rhs);

    uint8_t item_type;                // +0x00; projected as subgroup index
    uint8_t pad_01[3];
    int raw_04;
    int compare_key;                  // +0x08; the sole equality key
    int group_idx;                    // +0x0c
    int featured_idx;                 // +0x10
    int item_idx;                     // +0x14
    uint8_t raw_18;
    uint8_t pad_19[3];
    int stack_count;                  // +0x1c
    uint8_t sticker;                  // +0x20
    uint8_t pad_21;
    int16_t expiry_date;              // +0x22
    int raw_24;
    int raw_28;
    int gold_price;                   // +0x2c
    int win_point;                    // +0x30
    int mileage_price;                // +0x34
    int cera_price;                   // +0x38
    uint8_t raw_3c;
    uint8_t pad_3d[3];
    std::string raw_40;               // +0x40, the only non-POD field
    float bonus_weight;               // +0x44
    int raw_48;
    int raw_4c;
    uint8_t gift_disallowance;        // +0x50
    uint8_t cera_point_disallowance;  // +0x51
    uint8_t raw_52;
    uint8_t stack_buyable;            // +0x53
    int limit_sale_marker;            // +0x54; -2 means no sale limit
    int raw_58;
    int raw_5c;
    uint8_t raw_60;
    uint8_t pad_61;
    uint16_t raw_62;
    uint16_t raw_64;
    uint16_t pad_66;
    int raw_68;
    int raw_6c;
    int raw_70;
    int raw_74;
    uint8_t raw_78;
    uint8_t pad_79[3];
    int raw_7c;
    int raw_80;
    uint8_t raw_84;
    uint8_t pad_85[3];
};

// IDA unit: 07_npc_shop_dialog/CCeraShopGoods.
class CCeraShopGoods {
public:
    // IDA_EA: 0x8514EA0
    // IDA_NAME: _ZN14CCeraShopGoodsC2ERK14STCeraShopItem
    // DEMANGLED: CCeraShopGoods::CCeraShopGoods(STCeraShopItem const&)
    explicit CCeraShopGoods(const STCeraShopItem& item);

    // IDA_EA: 0x8510C1A
    // IDA_NAME: _ZN14CCeraShopGoods7SetItemERK14STCeraShopItem
    // DEMANGLED: CCeraShopGoods::SetItem(STCeraShopItem const&)
    CCeraShopGoods* SetItem(const STCeraShopItem& item);

    int GetItemIdx() const;
    int GetStackCount() const;
    int GetGoldPrice() const;
    int GetWinPoint() const;
    int GetCeraPrice() const;
    int GetMileagePrice() const;
    int GetSubGroupIndex() const;
    int GetGroupIdx() const;
    int GetFeaturedIdx() const;
    int GetSticker() const;
    int GetRelatedItem() const;
    int isGiftDisallowance_() const;
    int isCeraPointDisallowance_() const;
    int GetExpiryDate() const;
    int isStackBuyable() const;
    float GetBonusWeight() const;

private:
    int item_idx;
    int stack_count;
    int gold_price;
    int win_point;
    int cera_price;
    int mileage_price;
    int sub_group_index;
    int group_idx;
    int featured_idx;
    int sticker;
    int expiry_date;
    float bonus_weight;
    int raw_30;
    uint8_t gift_disallowance;
    uint8_t cera_point_disallowance;
    uint8_t stack_buyable;
    uint8_t pad_37;
    int related_item;
};

// IDA unit: 07_npc_shop_dialog/SECRET_SHOP_STATISTIC_DATA.
struct SECRET_SHOP_STATISTIC_DATA {
    SECRET_SHOP_STATISTIC_DATA();

    int item_id;
    int count;
    int raw_08;
    int raw_0c;
    int raw_10;
};

// IDA unit: 07_npc_shop_dialog/stCeraShopBonusItemInfo_t.
struct stCeraShopBonusItemInfo_t {
    ~stCeraShopBonusItemInfo_t();

    // The old item-list importer reads three integers for each record.  The
    // output chooser consumes item_index at +0, count at +4 and rate at +8.
    int item_index;
    uint16_t count;
    uint16_t reserved_06;
    int rate;
};

// IDA unit: 07_npc_shop_dialog/stCeraShopBonusInfo_t.
struct stCeraShopBonusInfo_t {
    stCeraShopBonusInfo_t();
    stCeraShopBonusInfo_t(const stCeraShopBonusInfo_t& rhs);
    ~stCeraShopBonusInfo_t();

    stCeraShopBonusInfo_t& operator=(const stCeraShopBonusInfo_t& rhs);

    int raw_00;
    int raw_04;
    std::vector<stCeraShopBonusItemInfo_t> bonus_items;
};

namespace WongWork {

namespace IPG {
struct SIPGData;
}  // namespace IPG

// IDA unit: 07_npc_shop_dialog/WongWork/CCeraShop simple fields.
class CCeraShop {
public:
    // IDA_EA: 0x83202B8
    // IDA_NAME: _ZN8WongWork9CCeraShopC2Ev
    // DEMANGLED: WongWork::CCeraShop::CCeraShop(void)
    CCeraShop();

    // IDA_EA: 0x8320314
    // IDA_NAME: _ZN8WongWork9CCeraShopD2Ev
    // DEMANGLED: WongWork::CCeraShop::~CCeraShop()
    ~CCeraShop();

    // IDA unit: 07_npc_shop_dialog/WongWork/CCeraShop.
    // Its two vectors are constructed at +0/+0x0c by C2 @0x817A244.
    struct stCeraShopItemParam_t {
        // IDA_EA: 0x817A244
        // IDA_NAME: _ZN8WongWork9CCeraShop21stCeraShopItemParam_tC2Ev
        // DEMANGLED: WongWork::CCeraShop::stCeraShopItemParam_t::stCeraShopItemParam_t(void)
        stCeraShopItemParam_t();

        // IDA_EA: 0x817A2FC
        // IDA_NAME: _ZN8WongWork9CCeraShop21stCeraShopItemParam_tD2Ev
        // DEMANGLED: WongWork::CCeraShop::stCeraShopItemParam_t::~stCeraShopItemParam_t()
        ~stCeraShopItemParam_t();

        std::vector<std::pair<int, int> > selected_items;
        std::vector<tagPackageItemSelection> package_item_selections;
    };

    // IDA unit: 07_npc_shop_dialog/WongWork/CCeraShop.  The input is not the
    // 28-byte DB record itself: the original copies its five semantic fields
    // into SIG_CERASHOP_ERROR_LOG while supplying the server/channel values.
    struct stErrorLogInput_t {
        uint32_t member_id;   // +0x00
        uint32_t charac_no;   // +0x04
        uint32_t error_code;  // +0x08
        int32_t goods_id;     // +0x0c
        int16_t type;         // +0x10
        uint16_t padding_12;  // +0x12, natural i386 alignment
    };

    static_assert(sizeof(stErrorLogInput_t) == 0x14u,
                  "CCeraShop::stErrorLogInput_t must stay 20 bytes");

    // IDA_EA: 0x832033A
    // IDA_NAME: _ZNK8WongWork9CCeraShop8LogErrorERKNS0_17stErrorLogInput_tE
    // DEMANGLED: WongWork::CCeraShop::LogError(WongWork::CCeraShop::stErrorLogInput_t const&)const
    int LogError(const stErrorLogInput_t& input) const;

    // IDA_EA: 0x832047A
    // IDA_NAME: _ZN8WongWork9CCeraShop17ProcessBuyRequestEP5CUserRK21MSG_BUY_CERASHOP_ITEM
    // DEMANGLED: WongWork::CCeraShop::ProcessBuyRequest(CUser *,MSG_BUY_CERASHOP_ITEM const&)
    int ProcessBuyRequest(CUser* user,
                          const MSG_BUY_CERASHOP_ITEM& message);

    // IDA_EA: 0x8321D2E
    // IDA_NAME: _ZN8WongWork9CCeraShop18ProcessIPGUserDataEP5CUserPNS_3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: WongWork::CCeraShop::ProcessIPGUserData(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    void ProcessIPGUserData(CUser* user,
                            IPG::SIPGData* request,
                            IPG::SIPGData* result,
                            Stream* callback_stream);

    // IDA_EA: 0x8323978
    // IDA_NAME: _ZN8WongWork9CCeraShop12ProcessErrorEP5CUsertijiic
    // DEMANGLED: WongWork::CCeraShop::ProcessError(CUser *,unsigned short,int,unsigned int,int,int,char)
    void ProcessError(CUser* user,
                      unsigned short error,
                      int result,
                      unsigned int goods_no,
                      int raw_1,
                      int raw_2,
                      char gift);

    // IDA_EA: 0x83237F6
    // IDA_NAME: _ZN8WongWork9CCeraShop16_ProcessIPGErrorEP5CUserPNS_3IPG8SIPGDataES5_hijiic
    // DEMANGLED: WongWork::CCeraShop::_ProcessIPGError(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,unsigned char,int,unsigned int,int,int,char)
    // The original source literal names this a void routine.  Hex-Rays
    // inferred an int only from transient EAX values left by its callees.
    void _ProcessIPGError(CUser* user,
                          IPG::SIPGData* request,
                          IPG::SIPGData* result,
                          unsigned char error,
                          int group_index,
                          unsigned int goods_no,
                          int item_order,
                          int item_count,
                          char gift);

    // IDA_EA: 0x8326036
    // IDA_NAME: _ZN8WongWork9CCeraShop18_processAvatarGiftEP5CUseriiccPKcji
    // DEMANGLED: WongWork::CCeraShop::_processAvatarGift(CUser *,int,int,char,char,char const*,unsigned int,int)
    void _processAvatarGift(CUser* user,
                            int item_id,
                            int usable_period,
                            char first_flag,
                            char avatar_select,
                            const char* order_id,
                            unsigned int gift_count,
                            int receiver_charac_no);

    // IDA_EA: 0x832619C
    // IDA_NAME: _ZN8WongWork9CCeraShop16_processCoinGiftEP5CUsertji
    // DEMANGLED: WongWork::CCeraShop::_processCoinGift(CUser *,unsigned short,unsigned int,int)
    int _processCoinGift(CUser* user,
                         unsigned short coin_count,
                         unsigned int gift_count,
                         int receiver_charac_no);

    // IDA_EA: 0x8326332
    // IDA_NAME: _ZN8WongWork9CCeraShop16_processCoinGiftEP5CUsertjii
    // DEMANGLED: WongWork::CCeraShop::_processCoinGift(CUser *,unsigned short,unsigned int,int,int)
    int _processCoinGift(CUser* user,
                         unsigned short coin_count,
                         unsigned int gift_count,
                         int receiver_charac_no,
                         int coin_item_id);

    // IDA_EA: 0x83264D6
    // IDA_NAME: _ZN8WongWork9CCeraShop16_processItemGiftEP5CUser10Inven_Itemji
    // DEMANGLED: WongWork::CCeraShop::_processItemGift(CUser *,Inven_Item,unsigned int,int)
    int _processItemGift(CUser* user,
                         Inven_Item item,
                         unsigned int gift_count,
                         int receiver_charac_no);

    // IDA_EA: 0x83265E8
    // IDA_NAME: _ZN8WongWork9CCeraShop20_processCreatureGiftEP5CUser10Inven_Itemjiii
    // DEMANGLED: WongWork::CCeraShop::_processCreatureGift(CUser *,Inven_Item,unsigned int,int,int,int)
    void _processCreatureGift(CUser* user,
                              Inven_Item item,
                              unsigned int gift_count,
                              int receiver_charac_no,
                              int usable_period,
                              int expiration_date);

    // IDA_EA: 0x8323F2A
    // IDA_NAME: _ZN8WongWork9CCeraShop20checkBuyingConditionEP5CUserjmjtht
    // DEMANGLED: WongWork::CCeraShop::checkBuyingCondition(CUser *,unsigned int,unsigned long,unsigned int,unsigned short,unsigned char,unsigned short)
    int checkBuyingCondition(CUser* user,
                             unsigned int ipg_no,
                             unsigned long related_item,
                             unsigned int related_item_count,
                             unsigned short avatar_type_select,
                             unsigned char raw_select,
                             unsigned short raw_select_2);

    int _CheckBuyingCondition(CUser* user,
                              unsigned int ipg_no,
                              unsigned long related_item,
                              unsigned int related_item_count,
                              unsigned short avatar_type_select,
                              unsigned char raw_select,
                              unsigned short raw_select_2);

    // IDA_EA: 0x8323AF4
    // IDA_NAME: _ZN8WongWork9CCeraShop16_CheckValidCountEPPK14CCeraShopGoods
    // DEMANGLED: WongWork::CCeraShop::_CheckValidCount(CCeraShopGoods const **)
    int _CheckValidCount(const CCeraShopGoods** goods);

    // IDA_EA: 0x8324810
    // IDA_NAME: _ZN8WongWork9CCeraShop21_CheckBuyingConditionEP5CUserPK14CCeraShopGoodsjhtcRi
    // DEMANGLED: WongWork::CCeraShop::_CheckBuyingCondition(CUser *,CCeraShopGoods const *,unsigned int,unsigned char,unsigned short,char,int &)
    int _CheckBuyingCondition(CUser* user,
                              const CCeraShopGoods* goods,
                              unsigned int ipg_no,
                              unsigned char raw_select,
                              unsigned short avatar_type_select,
                              char skip_coin_cap_check,
                              int& error);

    // IDA_EA: 0x8324966
    // IDA_NAME: _ZN8WongWork9CCeraShop26_CheckConditionPutonAvatarEP5CUser
    // DEMANGLED: WongWork::CCeraShop::_CheckConditionPutonAvatar(CUser *)
    int _CheckConditionPutonAvatar(CUser* user);

    // IDA_EA: 0x8324A1E
    // IDA_NAME: _ZN8WongWork9CCeraShop13IsSpecialItemEiRiS1_Rb
    // DEMANGLED: WongWork::CCeraShop::IsSpecialItem(int,int &,int &,bool &)
    int IsSpecialItem(int item_id,
                      int& special_type,
                      int& special_value,
                      bool& unique);

    // IDA_EA: 0x8325854
    // IDA_NAME: _ZN8WongWork9CCeraShop22CallSpecialItemHandlerEiRK19STSpecailItem_ParamR20STSpecailItem_Result
    // DEMANGLED: WongWork::CCeraShop::CallSpecialItemHandler(int,STSpecailItem_Param const&,STSpecailItem_Result&)
    int CallSpecialItemHandler(int special_item_type,
                               const STSpecailItem_Param& param,
                               STSpecailItem_Result& result);

    // IDA_EA: 0x832587E
    // IDA_NAME: _ZN8WongWork9CCeraShop18ProcessSpecialItemEP5CUseriiRilbRSt6vectorI10Inven_ItemSaIS5_EERNS0_21stCeraShopItemParam_tEccbi
    // DEMANGLED: WongWork::CCeraShop::ProcessSpecialItem(CUser *,int,int,int &,long,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,char,char,bool,int)
    int ProcessSpecialItem(CUser* user,
                           int item_id,
                           int restrict_code,
                           int& processing_result,
                           long current_seconds,
                           bool is_vending_machine,
                           std::vector<Inven_Item>& output_items,
                           stCeraShopItemParam_t& item_param,
                           char first_flag,
                           char second_flag,
                           bool third_flag,
                           int final_flag);

    // IDA_EA: 0x8325A88
    // IDA_NAME: _ZN8WongWork9CCeraShop12_processCoinEP5CUsertb
    // DEMANGLED: WongWork::CCeraShop::_processCoin(CUser *,unsigned short,bool)
    void _processCoin(CUser* user, unsigned short coin_count, bool is_gift);

    // IDA_EA: 0x8325B22
    // IDA_NAME: _ZN8WongWork9CCeraShop12_processItemEP5CUser10Inven_ItemiRilbbRSt6vectorIS3_SaIS3_EERNS0_21stCeraShopItemParam_tE
    // DEMANGLED: WongWork::CCeraShop::_processItem(CUser *,Inven_Item,int,int &,long,bool,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &)
    unsigned int _processItem(CUser* user,
                              Inven_Item item,
                              int special_item_id,
                              int& processing_result,
                              long current_seconds,
                              bool is_vending_machine,
                              bool save_inventory,
                              std::vector<Inven_Item>& output_items,
                              stCeraShopItemParam_t& item_param);

    // IDA_EA: 0x8325D66
    // IDA_NAME: _ZN8WongWork9CCeraShop16_processCreatureEP5CUser10Inven_ItemRiS4_bii
    // DEMANGLED: WongWork::CCeraShop::_processCreature(CUser *,Inven_Item,int &,int &,bool,int,int)
    int _processCreature(CUser* user,
                         Inven_Item item,
                         int& item_id,
                         int& error,
                         bool is_gift,
                         int usable_period,
                         int expiration_date);

    // IDA_EA: 0x83267A2
    // IDA_NAME: _ZN8WongWork9CCeraShop22_processAdditionalItemEP5CUsermthhRSt6vectorI10Inven_ItemSaIS4_EERNS0_21stCeraShopItemParam_tEh
    // DEMANGLED: WongWork::CCeraShop::_processAdditionalItem(CUser *,unsigned long,unsigned short,unsigned char,unsigned char,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,unsigned char)
    void _processAdditionalItem(CUser* user,
                                unsigned long item_id,
                                unsigned short item_count,
                                unsigned char avatar_option_0e,
                                unsigned char avatar_option_0d,
                                std::vector<Inven_Item>& output_items,
                                stCeraShopItemParam_t& item_param,
                                unsigned char equipment_upgrade);

    // IDA_EA: 0x8327894
    // IDA_NAME: _ZN8WongWork9CCeraShop21_processBoosterSelectEP5CUseriiRilbRSt6vectorI10Inven_ItemSaIS5_EERNS0_21stCeraShopItemParam_tEcc
    // DEMANGLED: WongWork::CCeraShop::_processBoosterSelect(CUser *,int,int,int &,long,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,char,char)
    int _processBoosterSelect(CUser* user,
                              int item_id,
                              int restrict_code,
                              int& processing_result,
                              long current_seconds,
                              bool is_vending_machine,
                              std::vector<Inven_Item>& output_items,
                              stCeraShopItemParam_t& item_param,
                              char first_flag,
                              char second_flag);

    // 140828 CMD160 carries the selected ability for every awarded avatar as
    // an item-id keyed table.  Keep the recovered 0627 entry point above for
    // its existing callers and expose a native request-specific owner instead
    // of changing the exported mangled ABI.
    int _processBoosterSelectWithAvatarOptions(
        CUser* user,
        int item_id,
        int restrict_code,
        int& processing_result,
        long current_seconds,
        bool is_vending_machine,
        std::vector<Inven_Item>& output_items,
        stCeraShopItemParam_t& item_param,
        char first_flag,
        char second_flag,
        const std::map<int, uint8_t>& avatar_options);

    // IDA_EA: 0x8325A30
    // IDA_NAME: _ZN8WongWork9CCeraShop15SaveFeaturedIdxEP5CUseriii
    // DEMANGLED: WongWork::CCeraShop::SaveFeaturedIdx(CUser *,int,int,int)
    void SaveFeaturedIdx(CUser* user,
                         int featured_index,
                         int current_seconds,
                         int source);

    // IDA_EA: 0x8327454
    // IDA_NAME: _ZN8WongWork9CCeraShop17_onBuySpecialItemEP5CUser21ENUM_CERA_SPECIALITEMm
    // DEMANGLED: WongWork::CCeraShop::_onBuySpecialItem(CUser *,ENUM_CERA_SPECIALITEM,unsigned long)
    int _onBuySpecialItem(CUser* user,
                          ENUM_CERA_SPECIALITEM special_item_type,
                          unsigned long item_id);

    // IDA_EA: 0x83275BE
    // IDA_NAME: _ZN8WongWork9CCeraShop27_sendInventoryUpgradePacketEP5CUseri
    // DEMANGLED: WongWork::CCeraShop::_sendInventoryUpgradePacket(CUser *,int)
    void _sendInventoryUpgradePacket(CUser* user, int special_item_type);

    // IDA_EA: 0x8327674
    // IDA_NAME: _ZN8WongWork9CCeraShop27_sendCharacSlotExtendPacketEP5CUseri
    // DEMANGLED: WongWork::CCeraShop::_sendCharacSlotExtendPacket(CUser *,int)
    void _sendCharacSlotExtendPacket(CUser* user, int special_item_type);

    // IDA_EA: 0x8327726
    // IDA_NAME: _ZN8WongWork9CCeraShop23_sendCargoUpgradePacketEP5CUseri
    // DEMANGLED: WongWork::CCeraShop::_sendCargoUpgradePacket(CUser *,int)
    void _sendCargoUpgradePacket(CUser* user, int special_item_type);

    // IDA_EA: 0x83277DC
    // IDA_NAME: _ZN8WongWork9CCeraShop18_sendPremiumPacketEP5CUseriii
    // DEMANGLED: WongWork::CCeraShop::_sendPremiumPacket(CUser *,int,int,int)
    void _sendPremiumPacket(CUser* user,
                            int special_item_type,
                            int premium_type,
                            int interval_seconds);

    // IDA_EA: 0x8327BB6
    // IDA_NAME: _ZN8WongWork9CCeraShop13_ClosePremiumEP5CUseri
    // DEMANGLED: WongWork::CCeraShop::_ClosePremium(CUser *,int)
    void _ClosePremium(CUser* user, int premium_type);

    CCeraShop* setOriginalItem(int item_id);
    int getOriginalItem();

    // IDA_EA: 0x8327F10
    // IDA_NAME: _ZN8WongWork9CCeraShop14autoUnsealItemEPK5CItemR10Inven_Item
    // DEMANGLED: WongWork::CCeraShop::autoUnsealItem(CItem const*,Inven_Item &)
    void autoUnsealItem(const CItem* item, Inven_Item& inventory_item);

    bool Is2010ChuseokPacket(unsigned int packet_id);

private:
    SpecialItemHandler* special_item_handler_;  // +0x00
    int original_item;                           // +0x04
};

}  // namespace WongWork

// IDA_EA: 0x8326739
// IDA_NAME: _Z14_isPackageItemPK5CItem
// DEMANGLED: _isPackageItem(CItem const*)
int _isPackageItem(const CItem* item);

// IDA unit: 99_misc_global/STSpecailItem_Param.
// C2 stores the two supplied references, clears the request state, and sets
// final_flag to one. The named fields below are the observed +0x00..+0x20 ABI.
struct STSpecailItem_Param {
    // IDA_EA: 0x8234EC2
    // IDA_NAME: _ZN19STSpecailItem_ParamC2ERSt6vectorI10Inven_ItemSaIS1_EERN8WongWork9CCeraShop21stCeraShopItemParam_tE
    // DEMANGLED: STSpecailItem_Param::STSpecailItem_Param(std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &)
    STSpecailItem_Param(
        std::vector<Inven_Item>& output_items,
        WongWork::CCeraShop::stCeraShopItemParam_t& item_param);

    CUser* user;                                           // +0x00
    int item_id;                                           // +0x04
    int special_value;                                     // +0x08
    std::vector<Inven_Item>* output_items;                 // +0x0c
    WongWork::CCeraShop::stCeraShopItemParam_t* item_param; // +0x10
    int restrict_code;                                     // +0x14
    long current_seconds;                                  // +0x18
    bool is_vending_machine;                               // +0x1c
    char first_flag;                                       // +0x1d
    char second_flag;                                      // +0x1e
    bool third_flag;                                       // +0x1f
    int final_flag;                                        // +0x20
};

// IDA unit: 99_misc_global/STSpecailItem_Result.
struct STSpecailItem_Result {
    // IDA_EA: 0x8234F36
    // IDA_NAME: _ZN20STSpecailItem_ResultC2Ev
    // DEMANGLED: STSpecailItem_Result::STSpecailItem_Result(void)
    STSpecailItem_Result();

    int result;  // +0x00
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(WongWork::CCeraShop::stCeraShopItemParam_t) == 0x18u,
              "CCeraShop item parameters must preserve two i386 vectors");
static_assert(sizeof(WongWork::CCeraShop) == 0x08u,
              "CCeraShop IDA ABI size must stay 0x08");
static_assert(sizeof(STSpecailItem_Param) == 0x24u,
              "STSpecailItem_Param IDA ABI size must stay 0x24");
static_assert(offsetof(STSpecailItem_Param, output_items) == 0x0cu,
              "STSpecailItem_Param output_items must stay at +0x0c");
static_assert(offsetof(STSpecailItem_Param, item_param) == 0x10u,
              "STSpecailItem_Param item_param must stay at +0x10");
static_assert(offsetof(STSpecailItem_Param, final_flag) == 0x20u,
              "STSpecailItem_Param final_flag must stay at +0x20");
static_assert(sizeof(STSpecailItem_Result) == 0x04u,
              "STSpecailItem_Result IDA ABI size must stay 0x04");
#endif


#endif  // OLDSRV_OLD_GAME_NPC_NPC_DIALOG_VALUE_TYPES_H_
