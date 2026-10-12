#include "old_game/economy/shop/npc_runtime.h"

#include <cmath>
#include <cstring>
#include <vector>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x817A2CE
// IDA_NAME: _ZN19Arad_MileageProcessC2EP18ItemVendingMachine
// DEMANGLED: Arad_MileageProcess::Arad_MileageProcess(ItemVendingMachine *)
Arad_MileageProcess::Arad_MileageProcess(
    ItemVendingMachine* vending_machine)
    : vending_machine_(vending_machine) {}

// IDA_EA: 0x817A2DC
// IDA_NAME: _ZN19Arad_MileageProcessD2Ev
// DEMANGLED: Arad_MileageProcess::~Arad_MileageProcess()
Arad_MileageProcess::~Arad_MileageProcess() {}

// IDA_EA: 0x819DC9C
// IDA_NAME: _ZN19Arad_MileageProcess11GiveMileageEP5CUserii
// DEMANGLED: Arad_MileageProcess::GiveMileage(CUser *,int,int)
void Arad_MileageProcess::GiveMileage(CUser* user,
                                      int goods_no,
                                      int cera_price) {
    if (!user) {
        return;
    }

    // `find` returns -1 when no explicit entry exists.  The original holds
    // that result in an unsigned register and therefore selects the script
    // default for every value greater than 1000, including -1.
    Arad_Mileage_Data* const mileage_data =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get()->getAradMileageData();
    const unsigned int configured_rate =
        static_cast<unsigned int>(mileage_data->find(goods_no));
    const int rate = configured_rate > 1000u
                         ? mileage_data->default_value()
                         : static_cast<int>(configured_rate);
    const int amount = static_cast<int>(
        std::floor(static_cast<long double>(cera_price) *
                   (static_cast<long double>(rate) / 1000.0L)));
    user->AddMileage(amount, static_cast<eCoinAddReason>(3));
}

// IDA_EA: 0x819DD4E
// IDA_NAME: _ZN19Arad_MileageProcess14BuyMileageItemEP5CUserimtilicciiiPKcRN8WongWork9CCeraShop21stCeraShopItemParam_tE
// DEMANGLED: Arad_MileageProcess::BuyMileageItem(CUser *,int,unsigned long,unsigned short,int,long,int,char,char,int,int,int,char const*,WongWork::CCeraShop::stCeraShopItemParam_t &)
int Arad_MileageProcess::BuyMileageItem(
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
    WongWork::CCeraShop::stCeraShopItemParam_t& item_param) {
    CCeraShopGoods* const goods = G_CDataManager()->FindGoods(goods_no);
    if (!goods) {
        return 21;
    }

    CItem* item_prototype = 0;
    Inven_Item item;
    if (goods->GetSubGroupIndex() != 1) {
        item_prototype = G_CDataManager()->find_item(static_cast<int>(item_id));
        if (!item_prototype) {
            return 21;
        }
        item_prototype->vtable_->make_item(item_prototype, item);
        item.item_id_ = static_cast<int32_t>(item_id);
        item.add_info_ = static_cast<int32_t>(item_count);
    }

    if (user->GetMileage() < cera_price) {
        return 1005;
    }

    int processing_result = 0;
    int output_item_id = 0;
    std::vector<Inven_Item> output_items;
    const int sub_group_index = goods->GetSubGroupIndex();
    switch (sub_group_index) {
    case 0: {
        const int slot = user->getCurCharacInvenW()->AddAvatarItem(
            static_cast<int>(item_id),
            avatar_selection_value,
            false,
            select_type,
            avatar_select,
            order_id,
            static_cast<eAvatarItemAddReason>(0),
            false,
            0);
        if (slot >= 0) {
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(1), slot);
        } else {
            const char* const mail_text = g_scriptStringManager_.findString(
                4, "game_server_msg_104", 0);
            WongWork::CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(
                user,
                user->getCurCharacNo(),
                static_cast<int>(item_id),
                avatar_selection_value,
                select_type,
                avatar_select,
                order_id,
                mail_text,
                static_cast<int>(std::strlen(mail_text)));
        }
        break;
    }
    case 1:
        vending_machine_->cera_shop_._processCoin(user, item_count, false);
        break;
    case 2:
    case 7:
    case 9:
        vending_machine_->cera_shop_._processItem(user,
                                                   item,
                                                   featured_index,
                                                   processing_result,
                                                   current_seconds,
                                                   false,
                                                   true,
                                                   output_items,
                                                   item_param);
        break;
    case 3:
        vending_machine_->cera_shop_._processCreature(
            user,
            item,
            output_item_id,
            processing_result,
            true,
            item_prototype->getUsablePeriod(),
            item_prototype->getExpirationDate());
        break;
    default:
        break;
    }

    if (processing_result == 0) {
        BuySuccess(user,
                   goods_no,
                   item_count,
                   output_item_id,
                   cera_price,
                   total_item_count,
                   item_order,
                   output_items);
        vending_machine_->BillingRecord(user,
                                        goods_no,
                                        sub_group_index,
                                        cera_price,
                                        order_id,
                                        "mileage item",
                                        item_count,
                                        0,
                                        0);
    }
    return processing_result;
}

// IDA_EA: 0x819E220
// IDA_NAME: _ZN19Arad_MileageProcess10BuySuccessEP5CUseriiiiiiRSt6vectorI10Inven_ItemSaIS3_EE
// DEMANGLED: Arad_MileageProcess::BuySuccess(CUser *,int,int,int,int,int,int,std::vector<Inven_Item,std::allocator<Inven_Item>> &)
void Arad_MileageProcess::BuySuccess(CUser* user,
                                     int goods_no,
                                     int item_count,
                                     int output_item_id,
                                     int cera_price,
                                     int total_item_count,
                                     int item_order,
                                     std::vector<Inven_Item>& output_items) {
    user->UseMileage(cera_price);

    PacketGuard result_packet;
    result_packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    result_packet.put_byte(1);
    result_packet.put_byte(0);
    result_packet.put_int(11);
    result_packet.put_int(goods_no);
    result_packet.put_int(item_count);
    result_packet.put_int(total_item_count);
    result_packet.put_int(output_item_id);
    result_packet.put_short(static_cast<int>(output_items.size()));
    for (std::vector<Inven_Item>::const_iterator it = output_items.begin();
         it != output_items.end(); ++it) {
        result_packet.put_int(it->item_id_);
        result_packet.put_int(it->add_info_);
    }

    if (total_item_count == item_order) {
        user->SendCashData(false);
        PacketGuard inventory_packet;
        inventory_packet.put_header(0, 14);
        inventory_packet.put_byte(0);
        inventory_packet.put_short(2);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 0, inventory_packet);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 2, inventory_packet);
        inventory_packet.finalize(true);
        user->Send(inventory_packet);
    }

    result_packet.finalize(true);
    user->Send(result_packet);
}

// IDA_EA: 0x819E50E
// IDA_NAME: _ZN19Arad_MileageProcess7BuyFailEP5CUsersiii
// DEMANGLED: Arad_MileageProcess::BuyFail(CUser *,short,int,int,int)
void Arad_MileageProcess::BuyFail(CUser* user,
                                  short error,
                                  int goods_no,
                                  int item_order,
                                  int total_item_count) {
    PacketGuard result_packet;
    result_packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    result_packet.put_byte(0);
    result_packet.put_byte(0);
    result_packet.put_short(error);
    result_packet.put_byte(0);
    result_packet.put_int(11);
    result_packet.put_int(goods_no);
    result_packet.put_int(item_order);
    result_packet.put_int(total_item_count);
    result_packet.put_int(-1);
    result_packet.finalize(true);
    user->Send(result_packet);
}
