#include "old_game/economy/mail/mail_trade_runtime.h"

#include <cstring>

#include "common/manager/os_api.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

namespace private_store {

// IDA_EA: 0x85CB972
// IDA_NAME: _ZN13private_store17CPacketBufPrivateC2Ev
CPacketBufPrivate::CPacketBufPrivate() : PacketGuard() {}

// IDA_EA: 0x85CB986
int CPacketBufPrivate::MakeCmdpacketCreatePrivateStore() {
    clear();
    put_header(1, ENUM_CMDPACKET_CREATE_PRIVATE_STORE);
    put_byte(1);
    return finalize(true);
}

// IDA_EA: 0x85CB9DA
int CPacketBufPrivate::MakeNotipacketCreatePrivateStore(
    uint16_t seller_index, PrivateStorePos& position, bool doll_created) {
    clear();
    put_header(0, 78);
    put_short(seller_index);
    put_byte(position.zone.village);
    put_byte(position.zone.area);
    put_short(position.x);
    put_short(position.y);
    put_byte(doll_created ? 1 : 0);
    return finalize(true);
}

// IDA_EA: 0x85CBAD8
int CPacketBufPrivate::MakeCmdpacketRemovePrivateStore() {
    clear();
    put_header(1, ENUM_CMDPACKET_REMOVE_PRIVATE_STORE);
    put_byte(1);
    return finalize(true);
}

// IDA_EA: 0x85CBB2C
int CPacketBufPrivate::MakeNotipacketRemovePrivateStore(uint16_t seller_index) {
    clear();
    put_header(0, 79);
    put_short(seller_index);
    return finalize(true);
}

// IDA_EA: 0x85CBB88
int CPacketBufPrivate::MakeNotipacketForceRemovePrivateStore() {
    clear();
    put_header(0, 80);
    return finalize(true);
}

// IDA_EA: 0x85CBBCA
int CPacketBufPrivate::MakeCmdpacketCompleteDisplay(bool doll_created) {
    clear();
    put_header(1, ENUM_CMDPACKET_COMPLETE_DISPLAY);
    put_byte(1);
    put_byte(doll_created ? 1 : 0);
    return finalize(true);
}

// IDA_EA: 0x85CBC52
int CPacketBufPrivate::MakeNotipacketCompleteDisplay(
    uint16_t seller_index, std::string& display_name, bool doll_created) {
    clear();
    put_header(0, 81);
    put_short(seller_index);
    put_int(static_cast<int>(display_name.length()));
    put_str(display_name.c_str(), static_cast<int>(display_name.length()));
    put_byte(doll_created ? 1 : 0);
    return finalize(true);
}

// IDA_EA: 0x85CBD38
int CPacketBufPrivate::MakeCmdpacketBuyPrivateStoreItem(
    CUser* user,
    Inven_Item& item,
    int /*arg4*/,
    int /*arg5*/,
    int slot,
    int item_add_info) {
    clear();
    put_header(1, ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM);
    put_byte(1);
    CInventory* const inventory = user->getCurCharacInvenR();
    put_int(inventory->get_money());
    put_int(user->GetWinPoint());
    put_int(user->GetCera());
    put_short(slot);
    put_int(item.item_id_);
    const int date_time_tick = OS_API::GetDateTimeTick();

    if (item.item_category_ == 8) {
        const WongWork::CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrR();
        put_int(avatar_manager->GetRemainDate(item_add_info,
                                               date_time_tick));
    } else {
        put_int(item_add_info);
    }

    put_byte(item.GetItemAttr());
    put_short(item.avatar_select_);
    put_int(item.extend_info_);
    put_byte(item.amplify_option_.getAbilityType());
    put_short(item.amplify_option_.getAbilityValue());
    if (item.item_category_ == 8) {
        put_int(30);
        const WongWork::CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrR();
        put_binary(reinterpret_cast<const char*>(
                       avatar_manager->getJewelSocketData(item_add_info)),
                   30);
        put_int(4);
        const WongWork::CAvatarItemMgr* const avatar_manager_for_expansion =
            inventory->GetAvatarItemMgrR();
        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager_for_expansion->GetExpansionInfo(item_add_info);
        if (expansion) {
            put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            put_binary(reinterpret_cast<const char*>(&empty_expansion), 4);
        }
    }
    put_packet(item);
    return finalize(true);
}

// IDA_EA: 0x85CBFEA
int CPacketBufPrivate::MakeNotipacketBuyPrivateStoreItem(
    CUser* user,
    int arg3,
    int slot,
    int arg5,
    const char* text,
    int money,
    int arg8) {
    clear();
    put_header(0, 82);
    put_byte(1);
    put_int(user->getCurCharacInvenR()->get_money());
    put_int(arg3);
    put_short(slot);
    put_int(arg5);
    const int text_length = static_cast<int>(std::strlen(text));
    put_int(text_length);
    put_str(text, text_length);
    put_int(money);
    put_int(arg8);
    return finalize(true);
}

// IDA_EA: 0x85CC0FA
int CPacketBufPrivate::MakeNotipacketPrivateStoreItem(int slot, int value) {
    clear();
    put_header(0, 83);
    put_short(slot);
    put_int(value);
    return finalize(true);
}

// IDA_EA: 0x85CC160
int CPacketBufPrivate::MakeCmdpacketEnterPrivateStore(
    CUser* user, uint16_t seller_index, std::vector<PrivateStoreItem>& items) {
    clear();
    put_header(1, ENUM_CMDPACKET_ENTER_PRIVATE_STORE);
    put_byte(1);
    put_short(seller_index);
    int count_index = get_index();
    put_byte(0);

    const int date_time_tick = OS_API::GetDateTimeTick();
    int count = 0;
    for (int index = 0; index <= 13; ++index) {
        PrivateStoreItem& stored_item = items[index];
        if (stored_item.GetState() != 3) continue;

        const Inven_Item& item = stored_item.item;
        put_short(index);
        put_int(stored_item.price);
        put_int(item.item_id_);
        if (item.item_category_ == 8) {
            const WongWork::CAvatarItemMgr* const avatar_manager =
                user->getCurCharacInvenR()->GetAvatarItemMgrR();
            put_int(avatar_manager->GetRemainDate(item.get_add_info(),
                                                   date_time_tick));
        } else {
            put_int(item.get_add_info());
        }
        put_byte(item.GetItemAttr());
        put_short(item.avatar_select_);
        put_int(item.extend_info_);
        put_byte(item.amplify_option_.getAbilityType());
        put_short(item.amplify_option_.getAbilityValue());
        if (item.item_category_ == 8) {
            put_int(30);
            const WongWork::CAvatarItemMgr* const avatar_manager =
                user->getCurCharacInvenR()->GetAvatarItemMgrR();
            put_binary(reinterpret_cast<const char*>(
                           avatar_manager->getJewelSocketData(item.get_add_info())),
                       30);
            put_int(4);
            const WongWork::CAvatarItemMgr* const
                avatar_manager_for_expansion =
                    user->getCurCharacInvenR()->GetAvatarItemMgrR();
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager_for_expansion->GetExpansionInfo(
                    item.get_add_info());
            if (expansion) {
                put_binary(reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                put_binary(reinterpret_cast<const char*>(&empty_expansion), 4);
            }
        }
        put_packet(item);
        ++count;
    }
    put_byte(count_index, count);
    return finalize(true);
}

// IDA_EA: 0x85CC568
int CPacketBufPrivate::MakeCmdpacketExitPrivateStore() {
    clear();
    put_header(1, ENUM_CMDPACKET_EXIT_PRIVATE_STORE);
    put_byte(1);
    return finalize(true);
}

// IDA_EA: 0x85CC5BC
int CPacketBufPrivate::MakeNotipacketPrivateStoreInfo(SIG_PRIVATE_STORE* payload) {
    clear();
    put_header(0, 85);
    put_byte(payload->loaded);
    int count_index = get_index();
    put_byte(0);

    int count = 0;
    for (int index = 0; index <= 13; ++index) {
        PrivateStoreItem& stored_item = payload->item[index];
        if (stored_item.GetState() != 3) continue;

        const Inven_Item& item = stored_item.item;
        put_short(index);
        put_int(stored_item.price);
        put_int(item.item_id_);
        put_int(item.get_add_info());
        put_byte(item.GetItemAttr());
        put_short(item.avatar_select_);
        put_int(item.extend_info_);
        put_byte(item.amplify_option_.getAbilityType());
        put_short(item.amplify_option_.getAbilityValue());
        put_packet(item);
        ++count;
    }
    put_byte(count_index, count);
    finalize(true);
    return count;
}

// IDA_EA: 0x85CC7DC
int CPacketBufPrivate::MakeCmdpacketMoveToGate() {
    clear();
    put_header(1, ENUM_CMDPACKET_MOVE_TO_GATE);
    put_byte(1);
    return finalize(true);
}

// IDA_EA: 0x85CC830
int CPacketBufPrivate::MakeErrorReturnPacket(
    int packet_class, int packet_id, int error_code) {
    clear();
    put_header(packet_class, packet_id);
    put_byte(0);
    put_byte(error_code);
    return finalize(true);
}

// IDA_EA: 0x85CCAAC
CPacketBufPrivate::~CPacketBufPrivate() {}

}  // namespace private_store
