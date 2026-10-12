#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/os_api.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x80CB0E8
// IDA_NAME: _Z14G_CDataManagerv
// DEMANGLED: G_CDataManager(void)
CDataManager* G_CDataManager();

namespace private_store {

// IDA_EA: 0x85C874C
// IDA_NAME: _ZN13private_store13CPrivateStore22VerifyPrivateStoreItemEii
// DEMANGLED: private_store::CPrivateStore::VerifyPrivateStoreItem(int,int)
int CPrivateStore::VerifyPrivateStoreItem(int slot, int requested_count) {
    error_handler.Set(buyer, 1, ENUM_CMDPACKET_VERIFY_PRIVATE_STORE_ITEM);
    if (!buyer) return 0;
    if (!manager) {
        error_handler.Do(58);
        return 0;
    }
    if (!IsExistBuyerList(buyer)) {
        error_handler.Do(21);
        return 0;
    }
    if (!buyer->getCurCharacR()) {
        error_handler.Do(1);
        return 0;
    }
    if (slot < 0 || slot >= 14) {
        error_handler.Do(57);
        return 0;
    }
    if (!seller || !seller->getCurCharacR()) {
        error_handler.Do(58);
        return 0;
    }

    PrivateStoreItem& store_item = items[slot];
    if (store_item.GetState() != 3) {
        error_handler.Do(1);
        return 0;
    }
    if (manager->IsBusyPrivateStore(buyer)) {
        error_handler.Do(60);
        return 0;
    }

    CItem* const data_item =
        G_CDataManager()->find_item(store_item.item.getKey());
    if (!data_item) {
        error_handler.Do(17);
        return 0;
    }
    const int attach_type = data_item->GetAttachType();
    if (attach_type == 3) {
        if (store_item.item.seal_or_state_ != 1) {
            error_handler.Do(17);
            return 0;
        }
    } else if (attach_type != 0) {
        if (attach_type == 7) {
            if (!store_item.item.GetTradeLimitCount()) return 0;
        } else {
            error_handler.Do(17);
            return 0;
        }
    }
    if (IsCreatureItemType(store_item.item.item_category_)) {
        error_handler.Do(17);
        return 0;
    }

    int money = 0;
    unsigned int first_value = 0;
    unsigned int second_value = 0;
    if (GetItemPrice(data_item, store_item, requested_count, money, first_value,
                     second_value) != 1 ||
        IsPossibleDeal(money) != 1) {
        return 0;
    }
    if (!buyer->GetReliablePerson() &&
        !buyer->IsAvailableCurCharacTradeGoldDaily(
            static_cast<unsigned int>(money))) {
        error_handler.Do(70);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_VERIFY_PRIVATE_STORE_ITEM);
    packet.put_byte(1);
    packet.put_short(slot);
    packet.put_int(store_item.price);
    const Inven_Item& item = store_item.item;
    packet.put_int(item.item_id_);
    const int date_time_tick = OS_API::GetDateTimeTick();
    if (item.item_category_ == 8) {
        const WongWork::CAvatarItemMgr* const avatar_manager =
            seller->getCurCharacInvenR()->GetAvatarItemMgrR();
        packet.put_int(avatar_manager->GetRemainDate(item.get_add_info(),
                                                      date_time_tick));
    } else {
        packet.put_int(requested_count);
    }
    packet.put_byte(item.GetItemAttr());
    packet.put_short(item.avatar_select_);
    packet.put_int(item.extend_info_);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    if (item.item_category_ == 8) {
        packet.put_int(30);
        const WongWork::CAvatarItemMgr* const avatar_manager =
            seller->getCurCharacInvenR()->GetAvatarItemMgrR();
        packet.put_binary(reinterpret_cast<const char*>(
                              avatar_manager->getJewelSocketData(item.get_add_info())),
                          30);
        const WongWork::CAvatarItemMgr* const avatar_manager_for_expansion =
            seller->getCurCharacInvenR()->GetAvatarItemMgrR();
        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager_for_expansion->GetExpansionInfo(item.get_add_info());
        packet.put_int(4);
        if (expansion) {
            packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet.put_binary(reinterpret_cast<const char*>(&empty_expansion), 4);
        }
    }
    packet.put_packet(item);
    packet.finalize(true);
    buyer->Send(packet);
    return 1;
}

}  // namespace private_store
