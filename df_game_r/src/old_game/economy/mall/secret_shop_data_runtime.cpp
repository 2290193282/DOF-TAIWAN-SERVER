// Canonical C-owned secret-shop purchase runtime.
#include "old_game/economy/mall/secret_shop_runtime.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/user.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/statistic_proxy_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "common/manager/mt_rand.h"
#include "infra/security/policy/secu_protection_field_abi.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <time.h>
#include <vector>

// IDA unit: 07_npc_shop_dialog/Packet_Secret_Shop_Statistic.  The binary
// starts the fixed 200-entry array at +0x12: a 10-byte PacketHeader is
// followed by the zero word at +0x0a and the NPC-position word at +0x0e.
#pragma pack(push, 1)
struct Packet_Secret_Shop_Statistic : public PacketHeader {
    Packet_Secret_Shop_Statistic();

    uint32_t zero_word;                 // +0x0a
    uint32_t npc_position;              // +0x0e
    SECRET_SHOP_STATISTIC_DATA data[200];  // +0x12
};
#pragma pack(pop)

static_assert(sizeof(Packet_Secret_Shop_Statistic) == 0xFB2u,
              "SecretShop statistic packet must retain its 0xfb2-byte wire ABI");

// IDA_EA: 0x85FD490
// IDA_NAME: _ZN28Packet_Secret_Shop_StatisticC2Ev
// DEMANGLED: Packet_Secret_Shop_Statistic::Packet_Secret_Shop_Statistic(void)
Packet_Secret_Shop_Statistic::Packet_Secret_Shop_Statistic()
    : PacketHeader(0x1B69u, 0x0FB2u), zero_word(0) {}

namespace secretshop {

namespace {

typedef void (*CItemInitializeFn)(CItem*, Inven_Item*);

int ItemCount(const Inven_Item& item) {
    return item.add_info_;
}

void SetItemId(Inven_Item& item, int item_id) {
    item.item_id_ = item_id;
}

void SetItemCount(Inven_Item& item, int count) {
    item.add_info_ = count;
}

}  // namespace

// IDA_EA: 0x85FBFC0
// IDA_NAME: _ZN10secretshop8IBuyRuleC2ERNS_20CSecretShopStatisticE
// DEMANGLED: secretshop::IBuyRule::IBuyRule(secretshop::CSecretShopStatistic &)
IBuyRule::IBuyRule(CSecretShopStatistic& statistic) : statistic_(&statistic) {}

// IDA_EA: 0x85FB55E
// IDA_NAME: _ZN10secretshop8IBuyRule21SendSecretShopBuyItemEP5CUseriR10Inven_Itemiii
// DEMANGLED: secretshop::IBuyRule::SendSecretShopBuyItem(CUser *,int,Inven_Item &,int,int,int)
void IBuyRule::SendSecretShopBuyItem(CUser* user,
                                     int inventory_slot,
                                     Inven_Item& item,
                                     int material_item_id,
                                     int material_count,
                                     int remaining_count) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM);
    packet.put_byte(1);
    CInventory* const inventory = static_cast<CInventory*>(
        user->getCurCharacInvenR());
    packet.put_int(inventory->get_money());
    packet.put_short(inventory_slot);
    packet.put_int(item.item_id_);
    packet.put_int(item.add_info_);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    packet.put_int(material_item_id);
    packet.put_int(material_count);
    packet.put_int(remaining_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85FB6C8
// IDA_NAME: _ZN10secretshop8IBuyRule23InsertItemIntoInventoryEP5CUseriiR10Inven_Item
// DEMANGLED: secretshop::IBuyRule::InsertItemIntoInventory(CUser *,int,int,Inven_Item &)
int IBuyRule::InsertItemIntoInventory(CUser* user,
                                      int item_id,
                                      int count,
                                      Inven_Item& item) {
    CItem* const source_item = G_CDataManager()->find_item(item_id);
    if (!source_item) {
        return -1;
    }

    void** const vtable = *reinterpret_cast<void***>(source_item);
    CItemInitializeFn initialize =
        reinterpret_cast<CItemInitializeFn>(vtable[2]);
    initialize(source_item, &item);
    SetItemId(item, item_id);
    if (source_item->is_stackable()) {
        SetItemCount(item, count);
    }

    CInventory* const inventory = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    const int inserted = inventory->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(44), true, false);
    return inserted >= 0 ? inserted : -1;
}

// IDA_EA: 0x85FB7E4
// IDA_NAME: _ZN10secretshop8IBuyRule10CheckLimitERNS_16SECRET_SHOP_INFOEiiRi
// DEMANGLED: secretshop::IBuyRule::CheckLimit(secretshop::SECRET_SHOP_INFO &,int,int,int &)
int IBuyRule::CheckLimit(SECRET_SHOP_INFO& user_shop,
                         int item_id,
                         int count,
                         int& remaining_count) {
    SALE_INFO* const sale_info = user_shop.GetSaleInfo(item_id);
    BUY_INFO* const buy_info = user_shop.shopper.GetBuyInfo(item_id);
    if (!sale_info) {
        return 0;
    }
    if (buy_info) {
        if (sale_info->value_0c < count + buy_info->count) {
            return 0;
        }
        remaining_count = sale_info->value_0c - (count + buy_info->count);
    } else {
        if (sale_info->value_0c < count) {
            return 0;
        }
        remaining_count = sale_info->value_0c - count;
    }
    return 1;
}

// IDA_EA: 0x85FB886
// IDA_NAME: _ZN10secretshop8IBuyRule17LogValueStatisticEP5CUserj
// DEMANGLED: secretshop::IBuyRule::LogValueStatistic(CUser *,unsigned int)
int IBuyRule::LogValueStatistic(CUser* user, unsigned int value) {
    return GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(13), user, value);
}

// IDA_EA: 0x85FB8B2
// IDA_NAME: _ZN10secretshop8IBuyRule16LogCubeStatisticEP5CUserRSt4pairIiiE
// DEMANGLED: secretshop::IBuyRule::LogCubeStatistic(CUser *,std::pair<int,int> &)
void IBuyRule::LogCubeStatistic(CUser* user,
                                std::pair<int, int>& item_and_count) {
    GetInstanceCubeStatistic()->collectCubeStatistics(
        item_and_count.first,
        item_and_count.second,
        user,
        static_cast<CUBE_STATISTIC_FIELD>(106));
}

// IDA_EA: 0x85FBFD8
// IDA_NAME: _ZN10secretshop14CBuyItembyGoldC2ERNS_20CSecretShopStatisticE
// DEMANGLED: secretshop::CBuyItembyGold::CBuyItembyGold(secretshop::CSecretShopStatistic &)
CBuyItembyGold::CBuyItembyGold(CSecretShopStatistic& statistic)
    : IBuyRule(statistic) {}

// IDA_EA: 0x85FB8F0
// IDA_NAME: _ZN10secretshop14CBuyItembyGold7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
// DEMANGLED: secretshop::CBuyItembyGold::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
int CBuyItembyGold::BuyItem(CUser* user,
                            SECRET_SHOP_INFO& user_shop,
                            int item_id,
                            int count) {
    SALE_INFO* const sale_info = user_shop.GetSaleInfo(item_id);
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(66));
    if (security_error) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }
    if (!sale_info) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 17);
        return 0;
    }

    int remaining_count = 0;
    if (CheckLimit(user_shop, item_id, count, remaining_count) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 95);
        return 0;
    }

    const unsigned int required_money = count * sale_info->price;
    if (user->getCurCharacMoney() <
        static_cast<int>(required_money)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 10);
        return 0;
    }

    Inven_Item item;
    const int inserted = InsertItemIntoInventory(user, item_id, count, item);
    if (inserted < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 4);
        return 0;
    }

    CInventory* const inventory = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    inventory->use_money(static_cast<int>(required_money),
                         static_cast<eMoneySubReason>(39),
                         true);
    SendSecretShopBuyItem(
        user, inserted, item, -1, 0, remaining_count);
    LogValueStatistic(user, required_money);
    user_shop.shopper.BuyItem(item_id, count);
    statistic_->RecordPrice(user->GetSecretShopData()->state,
                            user->getDungeonIdxAfterClear(),
                            static_cast<int>(required_money));
    return 1;
}

// IDA_EA: 0x85FBFFC
// IDA_NAME: _ZN10secretshop16CBuyItembyRecipeC2ERNS_20CSecretShopStatisticE
// DEMANGLED: secretshop::CBuyItembyRecipe::CBuyItembyRecipe(secretshop::CSecretShopStatistic &)
CBuyItembyRecipe::CBuyItembyRecipe(CSecretShopStatistic& statistic)
    : IBuyRule(statistic) {}

// IDA_EA: 0x85FBB62
// IDA_NAME: _ZN10secretshop16CBuyItembyRecipe7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
// DEMANGLED: secretshop::CBuyItembyRecipe::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
int CBuyItembyRecipe::BuyItem(CUser* user,
                              SECRET_SHOP_INFO& user_shop,
                              int item_id,
                              int count) {
    SALE_INFO* const sale_info = user_shop.GetSaleInfo(item_id);
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(67));
    if (security_error) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }
    if (!sale_info) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 17);
        return 0;
    }

    int remaining_count = 0;
    if (CheckLimit(user_shop, item_id, count, remaining_count) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 95);
        return 0;
    }

    const int material_item_id = sale_info->value_10;
    int required_material_count = count * sale_info->value_14;
    Inven_Item material_item;
    CInventory* const inventory_read = static_cast<CInventory*>(
        user->getCurCharacInvenR());
    const int material_slot =
        inventory_read->GetInvenData(material_item_id, material_item);
    if (material_slot == -1 || ItemCount(material_item) < required_material_count) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 10);
        return 0;
    }

    Inven_Item item;
    const int inserted = InsertItemIntoInventory(user, item_id, count, item);
    if (inserted < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM, 4);
        return 0;
    }

    CInventory* const inventory_write = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    inventory_write->delete_item(static_cast<INVEN_TYPE>(1),
                                 material_slot,
                                 required_material_count,
                                 static_cast<eItemDelReason>(26),
                                 true);
    const Inven_Item* const remaining_item = inventory_read->GetInvenRef(
        1, material_slot);
    if (remaining_item) {
        required_material_count = ItemCount(*remaining_item);
    } else {
        required_material_count = 0;
    }
    SendSecretShopBuyItem(user,
                          inserted,
                          item,
                          material_item_id,
                          required_material_count,
                          remaining_count);
    std::pair<int, int> material_pair(material_item_id, required_material_count);
    LogCubeStatistic(user, material_pair);
    user_shop.shopper.BuyItem(item_id, count);
    return 1;
}

// IDA_EA: 0x85FBEB8
// IDA_NAME: _ZN10secretshop20CSecretShopStatisticC2Ev
// DEMANGLED: secretshop::CSecretShopStatistic::CSecretShopStatistic(void)
CSecretShopStatistic::CSecretShopStatistic() {}

// IDA_EA: 0x85FBF2C
// IDA_NAME: _ZN10secretshop20CSecretShopStatisticD2Ev
// DEMANGLED: secretshop::CSecretShopStatistic::~CSecretShopStatistic()
CSecretShopStatistic::~CSecretShopStatistic() {
    for (int npc_pos = 0; npc_pos <= 2; ++npc_pos) {
        dungeon_data_[npc_pos].clear();
    }
}

// IDA_EA: 0x85FCFEC
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic5ClearEv
// DEMANGLED: secretshop::CSecretShopStatistic::Clear(void)
void CSecretShopStatistic::Clear() {
    for (int npc_pos = 0; npc_pos <= 2; ++npc_pos) {
        dungeon_data_[npc_pos].clear();
    }
}

// IDA_EA: 0x85FD024
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic9GetNpcPosEi
// DEMANGLED: secretshop::CSecretShopStatistic::GetNpcPos(int)
int CSecretShopStatistic::GetNpcPos(int npc_id) {
    switch (npc_id) {
    case 1002:
        return 0;
    case 1003:
        return 1;
    case 1004:
        return 2;
    }
    return 0;
}

// IDA_EA: 0x85FD05E
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic11GetNpcIndexEi
// DEMANGLED: secretshop::CSecretShopStatistic::GetNpcIndex(int)
int CSecretShopStatistic::GetNpcIndex(int npc_pos) {
    switch (npc_pos) {
    case 0:
        return 1002;
    case 1:
        return 1003;
    case 2:
        return 1004;
    }
    return 1002;
}

// IDA_EA: 0x85FD090
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic10RecordShowEiii
// DEMANGLED: secretshop::CSecretShopStatistic::RecordShow(int,int,int)
void CSecretShopStatistic::RecordShow(int npc_id,
                                      int dungeon_index,
                                      int member_count) {
    SECRET_SHOP_STATISTIC_DATA* data = GetDungeonData(npc_id, dungeon_index);
    ++data->count;
    data->raw_08 += member_count;
}

// IDA_EA: 0x85FD0D4
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic9RecordBuyEii
// DEMANGLED: secretshop::CSecretShopStatistic::RecordBuy(int npc_id,
//                                                          int dungeon_index)
void CSecretShopStatistic::RecordBuy(int npc_id, int dungeon_index) {
    ++GetDungeonData(npc_id, dungeon_index)->raw_0c;
}

// IDA_EA: 0x85FD108
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic11RecordPriceEiii
// DEMANGLED: secretshop::CSecretShopStatistic::RecordPrice(int,int,int)
void CSecretShopStatistic::RecordPrice(int npc_id,
                                       int dungeon_index,
                                       int price) {
    GetDungeonData(npc_id, dungeon_index)->raw_10 += price;
}

// IDA_EA: 0x85FD13E
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic14GetDungeonDataEii
// DEMANGLED: secretshop::CSecretShopStatistic::GetDungeonData(int,int)
SECRET_SHOP_STATISTIC_DATA* CSecretShopStatistic::GetDungeonData(
    int npc_id,
    int dungeon_index) {
    std::map<int, SECRET_SHOP_STATISTIC_DATA>& data_by_dungeon =
        dungeon_data_[GetNpcPos(npc_id)];
    std::map<int, SECRET_SHOP_STATISTIC_DATA>::iterator it =
        data_by_dungeon.find(dungeon_index);
    if (it == data_by_dungeon.end()) {
        SECRET_SHOP_STATISTIC_DATA data;
        data.item_id = dungeon_index;
        data_by_dungeon.insert(std::make_pair(dungeon_index, data));
        it = data_by_dungeon.find(dungeon_index);
    }
    return &it->second;
}

// IDA_EA: 0x85FD278
// IDA_NAME: _ZN10secretshop20CSecretShopStatistic23SendSecretShopStatisticEv
// DEMANGLED: secretshop::CSecretShopStatistic::SendSecretShopStatistic(void)
bool CSecretShopStatistic::SendSecretShopStatistic() {
    Packet_Secret_Shop_Statistic packet;

    for (int npc_pos = 0; npc_pos <= 2; ++npc_pos) {
        packet.npc_position = static_cast<uint32_t>(npc_pos);
        if (dungeon_data_[npc_pos].empty()) {
            continue;
        }

        std::memset(packet.data, 0, sizeof(packet.data));
        int record_index = 0;
        for (std::map<int, SECRET_SHOP_STATISTIC_DATA>::const_iterator it =
                 dungeon_data_[npc_pos].begin();
             it != dungeon_data_[npc_pos].end();
             ++it) {
            // IDA writes the map value sequentially into the fixed packet
            // array and relies on the script-side maximum of 200 entries.
            packet.data[record_index] = it->second;
            ++record_index;
        }

        CStatisticServerProxy* const proxy =
            GlobalData::s_statistic_proxy_mgr->GetServerProxy(
                SERVER_GROUP_DEFAULT);
        proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }
    return true;
}

// IDA_EA: 0x85FA930
// IDA_NAME: _ZN10secretshop11CSecretShopC2Ev
// DEMANGLED: secretshop::CSecretShop::CSecretShop(void)
CSecretShop::CSecretShop() : rules_(), random_(0), statistic_() {
    rules_.insert(std::make_pair(
        eBuyRule_Gold,
        static_cast<IBuyRule*>(new CBuyItembyGold(statistic_))));
    rules_.insert(std::make_pair(
        eBuyRule_Recipe,
        static_cast<IBuyRule*>(new CBuyItembyRecipe(statistic_))));
    const unsigned int seed_value = static_cast<unsigned int>(time(0));
    random_ = new CMTRand(seed_value);
}

// IDA_EA: 0x85FAAC4
// IDA_NAME: _ZN10secretshop11CSecretShopD2Ev
// DEMANGLED: secretshop::CSecretShop::~CSecretShop()
CSecretShop::~CSecretShop() {
    if (random_) {
        delete random_;
    }
    for (std::map<eBuyRule, IBuyRule*>::iterator it = rules_.begin();
         it != rules_.end();
         ++it) {
        ::operator delete(it->second);
    }
    rules_.clear();
}

// IDA_EA: 0x85FB53A
// IDA_NAME: _ZN10secretshop11CSecretShop23SendSecretShopStatisticEv
// DEMANGLED: secretshop::CSecretShop::SendSecretShopStatistic(void)
int CSecretShop::SendSecretShopStatistic() {
    statistic_.SendSecretShopStatistic();
    statistic_.Clear();
    return 1;
}

// IDA_EA: 0x85FABAC
// IDA_NAME: _ZN10secretshop11CSecretShop10LotteryNpcEiii
// DEMANGLED: secretshop::CSecretShop::LotteryNpc(int,int,int)
int CSecretShop::LotteryNpc(int dungeon_index,
                            int standard_level,
                            int member_count) {
    unsigned long n10000 = 10000;
    const int random_value = static_cast<int>(random_->randInt(n10000));
    int npc_index = 1000;
    SecretShopScript* const script = G_CDataManager()->get_secret_shop_script();
    if (script->GetNpcByDungeonIdx(npc_index, random_value, dungeon_index) != 1) {
        script->GetNpcByDungeonLev(npc_index, random_value, standard_level);
    }
    if (npc_index != 0 && npc_index != 1000) {
        statistic_.RecordShow(npc_index, dungeon_index, member_count);
    }
    return npc_index;
}

// IDA_EA: 0x85FAC68
// IDA_NAME: _ZN10secretshop11CSecretShop12LotteryItemsERSt6vectorINS_9SALE_INFOESaIS2_EEiii
// DEMANGLED: secretshop::CSecretShop::LotteryItems(std::vector<secretshop::SALE_INFO,std::allocator<secretshop::SALE_INFO>> &,int,int,int)
int CSecretShop::LotteryItems(std::vector<SALE_INFO>& items,
                              int npc_index,
                              int dungeon_index,
                              int standard_level) {
    // The native vtable call at +0x34 is CEventBase::IsEventing(CUser*) with
    // its original null-user argument.  Event 58 controls inclusion of the
    // event-sale table in both the index and level fallback paths.
    const bool include_event =
        GlobalData::s_event_manager->GetRepeatEvent(58)->IsEventing(0) != 0;
    SecretShopScript* const script = G_CDataManager()->get_secret_shop_script();
    const int from_dungeon = script->GetItemByDungeonIdx(
        random_, items, npc_index, dungeon_index, include_event);
    if (from_dungeon != 0) {
        return from_dungeon;
    }
    return script->GetItemByDungeonLev(
        random_, items, npc_index, standard_level, include_event);
}

// IDA_EA: 0x85FAD20
// IDA_NAME: _ZN10secretshop11CSecretShop12CheckLotteryEv
// DEMANGLED: secretshop::CSecretShop::CheckLottery(void)
int CSecretShop::CheckLottery() {
    std::vector<SALE_INFO> items;
    std::map<int, int> item_counts;
    FILE* const stream = std::fopen("secretshop_lottery.txt", "wt");
    if (!stream) {
        return 0;
    }

    // CDungeonList keeps the original __gnu_cxx::hash_map as its physical
    // member. Hold that native iterator for the full outer loop so the
    // diagnostic retains binary traversal order without repeatedly scanning
    // from begin() for every dungeon.
    CDungeonList* const dungeons = G_CDataManager()->getDungeonIndexList();
    for (CDungeonList::const_iterator dungeon_it = dungeons->begin();
         dungeon_it != dungeons->end();
         ++dungeon_it) {
        CDungeon* const dungeon = dungeon_it->second;
        if (!dungeon) {
            continue;
        }

        item_counts.clear();
        const int dungeon_index = dungeon->get_index();
        const int standard_level = dungeon->get_standard_level();
        for (int trial = 0; trial <= 9999; ++trial) {
            LotteryItems(items, 234, dungeon_index, standard_level);
            for (std::vector<SALE_INFO>::const_iterator item = items.begin();
                 item != items.end();
                 ++item) {
                ++item_counts[item->item_index];
            }
            items.clear();
        }

        std::fprintf(stream,
                     "Dungeon : %s(idx:%d, lev:%d)\n",
                     dungeon->GetDungeonName(),
                     dungeon_index,
                     standard_level);

        int total = 0;
        for (std::map<int, int>::const_iterator count = item_counts.begin();
             count != item_counts.end();
             ++count) {
            total += count->second;
        }
        for (std::map<int, int>::const_iterator count = item_counts.begin();
             count != item_counts.end();
             ++count) {
            const double ratio =
                static_cast<double>(count->second) / static_cast<double>(total) *
                100.0;
            std::fprintf(stream,
                         "- %d(%d, %.2f%%)\n",
                         count->first,
                         count->second,
                         ratio);
        }
        std::fputc('\n', stream);
    }

    std::fclose(stream);
    return 0;
}

// IDA_EA: 0x85FB16E
// IDA_NAME: _ZN10secretshop11CSecretShop7BuyItemEP5CUserRNS_16SECRET_SHOP_INFOEii
// DEMANGLED: secretshop::CSecretShop::BuyItem(CUser *,secretshop::SECRET_SHOP_INFO &,int,int)
int CSecretShop::BuyItem(CUser* user,
                         SECRET_SHOP_INFO& user_shop,
                         int item_id,
                         int count) {
    SALE_INFO* const sale_info = user_shop.GetSaleInfo(item_id);
    if (!sale_info) {
        return 0;
    }
    IBuyRule* const rule = GetRule(static_cast<eBuyRule>(sale_info->buy_rule));
    if (!rule) {
        return 0;
    }
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) {
        return 0;
    }
    if (item->is_stackable() != 1) {
        count = 1;
    }

    const int result = rule->BuyItem(user, user_shop, item_id, count);
    if (result && !user_shop.purchase_recorded) {
        SECRET_SHOP_DATA* const shop_data = user->GetSecretShopData();
        statistic_.RecordBuy(shop_data->state,
                             user->getDungeonIdxAfterClear());
        user_shop.purchase_recorded = 1;
    }
    return result;
}

// IDA_EA: 0x85FB27C
// IDA_NAME: _ZN10secretshop11CSecretShop7GetRuleE8eBuyRule
// DEMANGLED: secretshop::CSecretShop::GetRule(eBuyRule)
IBuyRule* CSecretShop::GetRule(eBuyRule rule) {
    std::map<eBuyRule, IBuyRule*>::iterator it = rules_.find(rule);
    if (it == rules_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x85FB2E0
// IDA_NAME: _ZN10secretshop11CSecretShop17SendSecretShopNpcEP5CUseri
// DEMANGLED: secretshop::CSecretShop::SendSecretShopNpc(CUser *,int)
void CSecretShop::SendSecretShopNpc(CUser* user, int npc_index) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 276);
    packet.put_int(npc_index);
    packet.finalize(true);
    const zombie_map_runtime::HookResult route =
        zombie_map_runtime::RouteBootstrapSecretShopPacket(user, packet);
    if (route == zombie_map_runtime::kHookNotManaged) user->Send(packet);
}

// IDA_EA: 0x85FB374
// IDA_NAME: _ZN10secretshop11CSecretShop22SendSecretShopItemListEP5CUserRSt6vectorINS_9SALE_INFOESaIS4_EE
// DEMANGLED: secretshop::CSecretShop::SendSecretShopItemList(CUser *,std::vector<secretshop::SALE_INFO,std::allocator<secretshop::SALE_INFO>> &)
void CSecretShop::SendSecretShopItemList(CUser* user,
                                         std::vector<SALE_INFO>& items) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 277);
    packet.put_int(static_cast<int>(items.size()));
    for (int index = 0; index < static_cast<int>(items.size()); ++index) {
        SALE_INFO& item = items[index];
        packet.put_int(item.item_index);
        packet.put_byte(item.buy_rule);
        packet.put_int(item.price);
        packet.put_int(item.value_0c);
        packet.put_int(item.value_10);
        packet.put_int(item.value_14);
        packet.put_byte(item.event_item);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82A5422
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATAC2Ev
// DEMANGLED: secretshop::SECRET_SHOP_DATA::SECRET_SHOP_DATA(void)
SECRET_SHOP_DATA::SECRET_SHOP_DATA() {}

// IDA_EA: 0x82A549E
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATAD2Ev
// DEMANGLED: secretshop::SECRET_SHOP_DATA::~SECRET_SHOP_DATA()
SECRET_SHOP_DATA::~SECRET_SHOP_DATA() {}

// IDA_EA: 0x8AB14B8
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA5clearEv
// DEMANGLED: secretshop::SECRET_SHOP_DATA::clear(void)
bool SECRET_SHOP_DATA::clear() {
    state = 0;
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 3;
        if (index > 3) {
            break;
        }
        infos[index].clear();
        buying[index] = 0;
    }
    return result;
}

// IDA_EA: 0x8AB150C
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA6IsOpenEv
// DEMANGLED: secretshop::SECRET_SHOP_DATA::IsOpen(void)
int SECRET_SHOP_DATA::IsOpen() {
    return state != 0 && state != 1000;
}

// IDA_EA: 0x8AB1532
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA9SetBuyingEib
// DEMANGLED: secretshop::SECRET_SHOP_DATA::SetBuying(int,bool)
unsigned int SECRET_SHOP_DATA::SetBuying(int user_pos, bool is_buying) {
    unsigned int result = static_cast<unsigned int>(is_buying);
    if (static_cast<unsigned int>(user_pos) < 4u) {
        buying[user_pos] = is_buying;
        result = static_cast<unsigned int>(user_pos);
    }
    return result;
}

// IDA_EA: 0x8AB155A
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_DATA13IsCompleteBuyEv
// DEMANGLED: secretshop::SECRET_SHOP_DATA::IsCompleteBuy(void)
int SECRET_SHOP_DATA::IsCompleteBuy() {
    for (int index = 0; index <= 3; ++index) {
        if (buying[index]) {
            return 0;
        }
    }
    return 1;
}

}  // namespace secretshop
