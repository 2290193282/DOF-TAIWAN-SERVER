#ifndef OLDSRV_OLD_GAME_NPC_NPC_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_NPC_RUNTIME_H_

#include <map>
#include <stdint.h>
#include <vector>

// CQuestShop is the active C-owned NPC/quest-shop gameplay class.
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/shop/npc_relation_event_manager.h"
#include "old_game/economy/mall/cerashop_purchase_bonus_runtime.h"
#include "old_game/economy/mall/quest_shop_runtime.h"

class CUser;
class CItem;
class Stream;
struct Inven_Item;
struct MSG_BUY_CERASHOP_ITEM;
class Arad_IPGProcess;

namespace WongWork {
namespace IPG {
struct SIPGData;
}  // namespace IPG
}  // namespace WongWork

// IDA unit: 07_npc_shop_dialog/CHackLog_InvitationPaperSellToNPC.
// Both functions are static members: the old Store call at 0x8619C43 passes
// only the item number, and the 0x8619C4C call passes no class instance.
class CHackLog_InvitationPaperSellToNPC {
public:
    // IDA_EA: 0x82866D0
    // IDA_NAME: _ZN33CHackLog_InvitationPaperSellToNPC17IsInvitationPaperEi
    // DEMANGLED: CHackLog_InvitationPaperSellToNPC::IsInvitationPaper(int)
    static int IsInvitationPaper(int item_id);

    // IDA_EA: 0x8286714
    // IDA_NAME: _ZN33CHackLog_InvitationPaperSellToNPC11GetHackTypeEv
    // DEMANGLED: CHackLog_InvitationPaperSellToNPC::GetHackType(void)
    static int GetHackType();
};

class Store {
public:
    // IDA_EA: 0x8618EA0
    // IDA_NAME: _ZN5Store16GetSellItemPriceER10Inven_ItemPK5CItemsbRi
    // DEMANGLED: Store::GetSellItemPrice(Inven_Item &,CItem const*,short,bool,int &)
    int GetSellItemPrice(Inven_Item& item,
                         const CItem* item_data,
                         short count,
                         bool special,
                         int& price);

    // IDA_EA: 0x8619008
    // IDA_NAME: _ZN5Store16GetSellItemPriceEP5CUsercssbRiS2_
    // DEMANGLED: Store::GetSellItemPrice(CUser *,char,short,short,bool,int &,int &)
    int GetSellItemPrice(CUser* user,
                         char item_space,
                         short slot,
                         short count,
                         bool dungeon_sale,
                         int& item_id,
                         int& price);

    // IDA_EA: 0x86193F8
    // IDA_NAME: _ZN5Store14user_sell_itemEP5CUsercss
    // DEMANGLED: Store::user_sell_item(CUser *,char,short,short)
    int user_sell_item(CUser* user,
                       char item_space,
                       short slot,
                       short count);

    // IDA_EA: 0x8619CEC
    // IDA_NAME: _ZN5Store12repair_equipEP5CUsercstRt
    // DEMANGLED: Store::repair_equip(CUser *,char,short,unsigned short,unsigned short &)
    int repair_equip(CUser* user,
                     char inventory_type,
                     short slot,
                     unsigned short repair_ticket_slot,
                     unsigned short& repair_cost);
};
class CNpc;
class ItemVendingMachine {
public:
    // IDA unit: 99_misc_global/ItemVendingMachine::AuctionNeedInfo.
    // The complete 0x28-byte definition stays with the Taiwan billing ABI in
    // item_vending_machine_auction_runtime.h so this public NPC header does
    // not acquire a hard dependency on the billing implementation layout.
    struct AuctionNeedInfo;

    // IDA_EA: 0x854C3E4
    // IDA_NAME: _ZN18ItemVendingMachineC2Ev
    // DEMANGLED: ItemVendingMachine::ItemVendingMachine(void)
    ItemVendingMachine();

    // IDA_EA: 0x854C42A
    // IDA_NAME: _ZN18ItemVendingMachineD2Ev
    // DEMANGLED: ItemVendingMachine::~ItemVendingMachine()
    ~ItemVendingMachine();

    int ProcessBuyRequestEx(CUser* user,
                            const MSG_BUY_CERASHOP_ITEM& message);

    // IDA_EA: 0x854C4D0
    // IDA_NAME: _ZN18ItemVendingMachine7BuyItemEP5CUserRK21MSG_BUY_CERASHOP_ITEM
    // DEMANGLED: ItemVendingMachine::BuyItem(CUser *,MSG_BUY_CERASHOP_ITEM const&)
    int BuyItem(CUser* user, const MSG_BUY_CERASHOP_ITEM& message);

    // IDA_EA: 0x854C4F6
    // IDA_NAME: _ZN18ItemVendingMachine18ProcessIPGUserDataEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: ItemVendingMachine::ProcessIPGUserData(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    int ProcessIPGUserData(CUser* user,
                           WongWork::IPG::SIPGData* request,
                           WongWork::IPG::SIPGData* result,
                           Stream* callback_stream);

    // IDA_EA: 0x81783EA
    // IDA_NAME: _ZN18ItemVendingMachine23ProcessIPGUserDataJpnExEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: ItemVendingMachine::ProcessIPGUserDataJpnEx(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    void ProcessIPGUserDataJpnEx(CUser* user,
                                 WongWork::IPG::SIPGData* request,
                                 WongWork::IPG::SIPGData* result,
                                 Stream* callback_stream);

    // IDA_EA: 0x81784D4
    // IDA_NAME: _ZN18ItemVendingMachine22ProcessIPG_ResultQueryEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: ItemVendingMachine::ProcessIPG_ResultQuery(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    void ProcessIPG_ResultQuery(CUser* user,
                                WongWork::IPG::SIPGData* request,
                                WongWork::IPG::SIPGData* result,
                                Stream* callback_stream);

    // IDA_EA: 0x817866C
    // IDA_NAME: _ZN18ItemVendingMachine22ProcessIPG_ResultInputEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: ItemVendingMachine::ProcessIPG_ResultInput(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    void ProcessIPG_ResultInput(CUser* user,
                                WongWork::IPG::SIPGData* request,
                                WongWork::IPG::SIPGData* result,
                                Stream* callback_stream);

    // IDA_EA: 0x8178676
    // IDA_NAME: _ZN18ItemVendingMachine23ProcessIPG_ResultOutputEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
    // DEMANGLED: ItemVendingMachine::ProcessIPG_ResultOutput(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
    int ProcessIPG_ResultOutput(CUser* user,
                                WongWork::IPG::SIPGData* request,
                                WongWork::IPG::SIPGData* result,
                                Stream* callback_stream);

    // IDA_EA: 0x817991E
    // IDA_NAME: _ZN18ItemVendingMachine13BillingRecordEP5CUseriiiPKcS3_iyc
    // DEMANGLED: ItemVendingMachine::BillingRecord(CUser *,int,int,int,char const*,char const*,int,unsigned long long,char)
    void BillingRecord(CUser* user,
                       int goods_no,
                       int group_index,
                       int cera_price,
                       const char* order_id,
                       const char* product_name,
                       int stack_count,
                       unsigned long long transaction_id,
                       char gift);

    // IDA_EA: 0x8179EAE
    // IDA_NAME: _ZN18ItemVendingMachine23LoadPurcahseBonusScriptEv
    // DEMANGLED: ItemVendingMachine::LoadPurcahseBonusScript(void)
    int LoadPurcahseBonusScript();

    // IDA_EA: 0x8179ED8
    // IDA_NAME: _ZN18ItemVendingMachine22CheckPurcahseBonusItemEm
    // DEMANGLED: ItemVendingMachine::CheckPurcahseBonusItem(unsigned long)
    int CheckPurcahseBonusItem(unsigned long goods_no);

    // IDA_EA: 0x8179F08
    // IDA_NAME: _ZN18ItemVendingMachine24ProcessPurcahseBonusItemEP5CUsermit
    // DEMANGLED: ItemVendingMachine::ProcessPurcahseBonusItem(CUser *,unsigned long,int,unsigned short)
    int ProcessPurcahseBonusItem(CUser* user,
                                 unsigned long purchase_index,
                                 int goods_no,
                                 unsigned short purchase_count);

    // IDA_EA: 0x8175C5C
    // IDA_NAME: _ZN18ItemVendingMachine14BuyAuctionItemEP5CUserRNS_15AuctionNeedInfoEibi
    // DEMANGLED: ItemVendingMachine::BuyAuctionItem(CUser *,ItemVendingMachine::AuctionNeedInfo &,int,bool,int)
    bool BuyAuctionItem(CUser* user,
                        AuctionNeedInfo& need_info,
                        int billing_context,
                        bool is_vip,
                        int tax_divisor);

private:
    friend class Arad_IPGProcess;
    friend class Arad_MileageProcess;

    WongWork::CCeraShop cera_shop_;                         // +0x00
    ARAD::CeraShopPurcahseCountBonus purchase_count_bonus_; // +0x08
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ItemVendingMachine) == 0x20u,
              "ItemVendingMachine IDA ABI size must stay 0x20");
#endif

// IDA unit: 99_misc_global/Arad_IPGProcess.  The original object is a
// single ItemVendingMachine pointer; its methods are the typed group 0..3
// completion paths used by ItemVendingMachine::ProcessIPG_ResultOutput.
class Arad_IPGProcess {
public:
    // IDA_EA: 0x817A2BA
    // IDA_NAME: _ZN15Arad_IPGProcessC2EP18ItemVendingMachine
    // DEMANGLED: Arad_IPGProcess::Arad_IPGProcess(ItemVendingMachine *)
    explicit Arad_IPGProcess(ItemVendingMachine* vending_machine);

    // IDA_EA: 0x817A2C8
    // IDA_NAME: _ZN15Arad_IPGProcessD2Ev
    // DEMANGLED: Arad_IPGProcess::~Arad_IPGProcess()
    ~Arad_IPGProcess();

    // IDA_EA: 0x819C2B4
    // IDA_NAME: _ZN15Arad_IPGProcess18CashShopModeAvatarEP5CUsermiiccPKcRijic
    // DEMANGLED: Arad_IPGProcess::CashShopModeAvatar(CUser *,unsigned long,int,int,char,char,char const*,int &,unsigned int,int,char)
    void CashShopModeAvatar(CUser* user,
                            unsigned long item_id,
                            int goods_no,
                            int usable_period,
                            char ability_no,
                            char avatar_select,
                            const char* order_id,
                            int& processing_result,
                            unsigned int receiver_member_id,
                            int receiver_charac_no,
                            char gift);

    // IDA_EA: 0x819C434
    // IDA_NAME: _ZN15Arad_IPGProcess16CashShopModeCoinEP5CUsertjic
    // DEMANGLED: Arad_IPGProcess::CashShopModeCoin(CUser *,unsigned short,unsigned int,int,char)
    int CashShopModeCoin(CUser* user,
                         unsigned short coin_count,
                         unsigned int receiver_member_id,
                         int receiver_charac_no,
                         char gift);

    // IDA_EA: 0x819C4A0
    // IDA_NAME: _ZN15Arad_IPGProcess16CashShopModeItemEP5CUsermtilRSt6vectorI10Inven_ItemSaIS3_EERN8WongWork9CCeraShop21stCeraShopItemParam_tERijic
    // DEMANGLED: Arad_IPGProcess::CashShopModeItem(CUser *,unsigned long,unsigned short,int,long,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,int &,unsigned int,int,char)
    unsigned int CashShopModeItem(
        CUser* user,
        unsigned long item_id,
        unsigned short item_count,
        int special_item_id,
        long current_seconds,
        std::vector<Inven_Item>& output_items,
        WongWork::CCeraShop::stCeraShopItemParam_t& item_param,
        int& processing_result,
        unsigned int receiver_member_id,
        int receiver_charac_no,
        char gift);

    // IDA_EA: 0x819C670
    // IDA_NAME: _ZN15Arad_IPGProcess20CashShopModeCreatureEP5CUsermtRiS2_jic
    // DEMANGLED: Arad_IPGProcess::CashShopModeCreature(CUser *,unsigned long,unsigned short,int &,int &,unsigned int,int,char)
    // 0x819C670 only drives the two by-reference outputs.  Its sole caller
    // (0x8178BE5) discards EAX, and the gift branch calls the void
    // _processCreatureGift implementation.  Do not expose the decompiler's
    // inferred CItem* return: it would turn the normal branch's slot number
    // into a fabricated pointer.
    void CashShopModeCreature(CUser* user,
                              unsigned long item_id,
                              unsigned short item_count,
                              int& output_item_id,
                              int& processing_result,
                              unsigned int receiver_member_id,
                              int receiver_charac_no,
                              char gift);

private:
    ItemVendingMachine* vending_machine_;  // +0x00
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_IPGProcess) == 0x04u,
              "Arad_IPGProcess IDA ABI size must stay one pointer");
#endif

// IDA unit: 99_misc_global/Arad_MileageProcess.  It is the companion
// one-pointer transaction object used by the group-11 CeraShop result path.
// Keeping it separate from Arad_IPGProcess preserves the original static-local
// lifetime and all of its purchase/result packet branches.
class Arad_MileageProcess {
public:
    // IDA_EA: 0x817A2CE
    // IDA_NAME: _ZN19Arad_MileageProcessC2EP18ItemVendingMachine
    // DEMANGLED: Arad_MileageProcess::Arad_MileageProcess(ItemVendingMachine *)
    explicit Arad_MileageProcess(ItemVendingMachine* vending_machine);

    // IDA_EA: 0x817A2DC
    // IDA_NAME: _ZN19Arad_MileageProcessD2Ev
    // DEMANGLED: Arad_MileageProcess::~Arad_MileageProcess()
    ~Arad_MileageProcess();

    // IDA_EA: 0x819DC9C
    // IDA_NAME: _ZN19Arad_MileageProcess11GiveMileageEP5CUserii
    // DEMANGLED: Arad_MileageProcess::GiveMileage(CUser *,int,int)
    void GiveMileage(CUser* user, int goods_no, int cera_price);

    // IDA_EA: 0x819DD4E
    // IDA_NAME: _ZN19Arad_MileageProcess14BuyMileageItemEP5CUserimtilicciiiPKcRN8WongWork9CCeraShop21stCeraShopItemParam_tE
    // DEMANGLED: Arad_MileageProcess::BuyMileageItem(CUser *,int,unsigned long,unsigned short,int,long,int,char,char,int,int,int,char const*,WongWork::CCeraShop::stCeraShopItemParam_t &)
    int BuyMileageItem(
        CUser* user,
        int goods_no,
        unsigned long item_id,
        unsigned short item_count,
        int featured_index,
        long current_seconds,
        int avatar_selection_value,
        char select_type,
        char avatar_select,
        int cera_price,
        int total_item_count,
        int item_order,
        const char* order_id,
        WongWork::CCeraShop::stCeraShopItemParam_t& item_param);

    // IDA_EA: 0x819E220
    // IDA_NAME: _ZN19Arad_MileageProcess10BuySuccessEP5CUseriiiiiiRSt6vectorI10Inven_ItemSaIS3_EE
    // DEMANGLED: Arad_MileageProcess::BuySuccess(CUser *,int,int,int,int,int,int,std::vector<Inven_Item,std::allocator<Inven_Item>> &)
    void BuySuccess(CUser* user,
                    int goods_no,
                    int item_count,
                    int output_item_id,
                    int cera_price,
                    int total_item_count,
                    int item_order,
                    std::vector<Inven_Item>& output_items);

    // IDA_EA: 0x819E50E
    // IDA_NAME: _ZN19Arad_MileageProcess7BuyFailEP5CUsersiii
    // DEMANGLED: Arad_MileageProcess::BuyFail(CUser *,short,int,int,int)
    void BuyFail(CUser* user,
                 short error,
                 int goods_no,
                 int item_order,
                 int total_item_count);

private:
    ItemVendingMachine* vending_machine_;  // +0x00
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_MileageProcess) == 0x04u,
              "Arad_MileageProcess IDA ABI size must stay one pointer");
#endif

class CNpcShop;
class CVillageObjectMgr;
class CVillageMonsterMgr;

#endif  // OLDSRV_OLD_GAME_NPC_NPC_RUNTIME_H_
