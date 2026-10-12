#include "old_game/appearance/pet/creature.h"

#include "old_game/economy/inventory/inventory.h"

namespace WongWork {

// IDA_EA: 0x82F913E
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr18updateEmblemSocketEijPcPm
// DEMANGLED: WongWork::CAvatarItemMgr::updateEmblemSocket(int,unsigned int,char *,unsigned long *)
int CAvatarItemMgr::updateEmblemSocket(
    int key,
    unsigned int socket_count,
    char* socket_types,
    unsigned long* emblem_item_ids) {
    Avatar_Item* const item = _FindItem(key);
    if (!item) {
        return 0;
    }

    for (unsigned int index = 0; index < socket_count; ++index) {
        // 0x82F9183/0x82F9186 performs movzx followed by movsx before the
        // unsigned Avatar_Item callee receives the socket value.
        item->updateEmblemSocket(
            static_cast<unsigned int>(
                static_cast<signed char>(socket_types[index])),
            static_cast<unsigned int>(emblem_item_ids[index]));
    }
    AddEmblemEndurance(key, item);
    return 1;
}

// IDA_EA: 0x82F95F4
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr27RegistNotAssignedAvatarItemE10Inven_Item
// DEMANGLED: WongWork::CAvatarItemMgr::RegistNotAssignedAvatarItem(Inven_Item)
int CAvatarItemMgr::RegistNotAssignedAvatarItem(Inven_Item item) {
    const int temporary_key = _AssignTempKey();
    Avatar_Item* const avatar_item = m_AvatarItemPool.Acquire();

    // The original reads the by-value Inven_Item at its +0x07 add_info_ field.
    avatar_item->SetData(item.add_info_, 0, "");
    avatar_item->setColor(0, 0);
    wait_assign_items_.insert(std::make_pair(temporary_key, avatar_item));
    return temporary_key;
}

// IDA_EA: 0x82F96A6
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr20AssignAvatarItemInfoEiiiiPKc
// DEMANGLED: WongWork::CAvatarItemMgr::AssignAvatarItemInfo(int,int,int,int,char const*)
int CAvatarItemMgr::AssignAvatarItemInfo(int temporary_key,
                                         int avatar_ui_id,
                                         int expire_date,
                                         int regist_time,
                                         const char* agency_no) {
    Avatar_Item* const item = _FindWaitAssignItem(temporary_key);
    if (!item) {
        return 0;
    }

    item->SetData(expire_date, regist_time, agency_no);
    wait_assign_items_.erase(temporary_key);
    assigned_items_.insert(std::make_pair(avatar_ui_id, item));
    AddEmblemEndurance(avatar_ui_id, item);
    return 1;
}

// IDA_EA: 0x82F988C
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetRegistTimeEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetRegistTime(int)const
int CAvatarItemMgr::GetRegistTime(int key) const {
    Avatar_Item* const item = _FindItemAll(key);
    return item ? item->GetRegistTime() : -1;
}

}  // namespace WongWork
