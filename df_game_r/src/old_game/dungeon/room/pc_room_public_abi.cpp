#include "old_game/dungeon/room/pc_room_public_abi.h"

#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>

namespace {

const int kPcRoomRentalType = 1;
const int kDimensionRentalType = 2;

Inven_Item::ITEM_TYPE ItemType(int value) {
    return static_cast<Inven_Item::ITEM_TYPE>(value);
}

INVEN_TYPE InventoryType(int value) {
    return static_cast<INVEN_TYPE>(value);
}

eItemAddReason ItemAddReason(int value) {
    return static_cast<eItemAddReason>(value);
}

void FillRentalInfo(InstanceRentalSystem::RentalInfo& rental_info,
                    const InstanceRentalSystem::RentInput::RentInElem& input,
                    const EventRewardItemInfo& item_info,
                    int slot,
                    int item_add_info,
                    bool equipment) {
    rental_info.state = equipment ? 3u : 0u;
    rental_info.slot = static_cast<uint16_t>(slot);
    rental_info.item_id = item_info.first_dword;
    rental_info.section_index = input.section_index;
    rental_info.package_index = input.package_index;
    rental_info.equip_flag = input.equip_flag;
    rental_info.rent_type = input.rent_type;
    rental_info.expire_time =
        input.expire_time == -1 ? item_add_info : input.expire_time;
}

}  // namespace

namespace pc_room {

// IDA_EA: 0x8270FEA
// IDA_NAME: _ZN7pc_room12IsPcRoomUserER5CUser
// DEMANGLED: pc_room::IsPcRoomUser(CUser &)
int IsPcRoomUser(CUser& user) { return user.GetPCRoomNo() != 0; }

// IDA_EA: 0x8271002
// IDA_NAME: _ZN7pc_room24CheckRentableEnvironmentER5CUser
// DEMANGLED: pc_room::CheckRentableEnvironment(CUser &)
int CheckRentableEnvironment(CUser&) {
    const int channel_type = G_GameWorld()->GetChannelType();
    return channel_type != 15 && channel_type != 16;
}

// IDA_EA: 0x8271032
// IDA_NAME: _ZN7pc_room37CheckRentableEnvironmentAtChannelTypeER5CUser
// DEMANGLED: pc_room::CheckRentableEnvironmentAtChannelType(CUser &)
int CheckRentableEnvironmentAtChannelType(CUser&) {
    const int channel_type = G_GameWorld()->GetChannelType();
    return channel_type != 15 && channel_type != 16;
}

// IDA_EA: 0x8271062
// IDA_NAME: _ZN7pc_room33CheckInventoryEquipmentEmptyCountER5CUseri
// DEMANGLED: pc_room::CheckInventoryEquipmentEmptyCount(CUser &,int)
int CheckInventoryEquipmentEmptyCount(CUser& user, int count) {
    return user.getCurCharacInvenR()->check_empty_count(ItemType(1), count);
}

// IDA_EA: 0x827108C
// IDA_NAME: _ZN7pc_room24CheckInventoryEmptyCountER5CUseri
// DEMANGLED: pc_room::CheckInventoryEmptyCount(CUser &,int)
int CheckInventoryEmptyCount(CUser& user, int count) {
    CInventory* const inventory = user.getCurCharacInvenR();
    const int checked_types[] = {1, 2, 3, 10, 9, 5, 6};
    for (unsigned int i = 0; i != sizeof(checked_types) / sizeof(checked_types[0]);
         ++i) {
        if (inventory->check_empty_count(ItemType(checked_types[i]), count) !=
            1) {
            return 1;
        }
    }
    return inventory->GetRemainCapacity(InventoryType(2), ItemType(8)) >= count
               ? 1
               : 0;
}

// IDA_EA: 0x82711F7
// IDA_NAME: _ZN7pc_room17GetSelectionIndexER5CUserRii
// DEMANGLED: pc_room::GetSelectionIndex(CUser &,int &,int)
int GetSelectionIndex(CUser& user, int& selection_index, int rental_type) {
    if (!user.getCurCharacR()) {
        return 0;
    }
    int pvp_grade = user.get_pvp_grade();
    if (user.isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(22))) {
        pvp_grade += 5;
    }
    if (rental_type == kDimensionRentalType) {
        pvp_grade = user.get_pvp_grade();
    }
    EventRewardSection* const section =
        GetRewardSection(std::max(0, pvp_grade), rental_type);
    if (!section) {
        return 0;
    }
    selection_index = section->byte_00;
    return 1;
}

// IDA_EA: 0x82712A6
// IDA_NAME: _ZN7pc_room15GetCharacterJobER5CUserRc
// DEMANGLED: pc_room::GetCharacterJob(CUser &,char &)
int GetCharacterJob(CUser& user, char& character_job) {
    if (!user.getCurCharacR()) {
        return 0;
    }
    character_job = static_cast<char>(user.get_charac_job());
    return 1;
}

// IDA_EA: 0x82712DA
// IDA_NAME: _ZN7pc_room26GetRewardItemSelectionListEii
// DEMANGLED: pc_room::GetRewardItemSelectionList(int,int)
std::vector<std::vector<EventRewardItemInfo> >* GetRewardItemSelectionList(
    int package_key,
    int rental_type) {
    if (rental_type == kPcRoomRentalType) {
        return CDataManagerPcBangItemRentarData(G_CDataManager())
            ->getRewardSelectionList(package_key);
    }
    if (rental_type == kDimensionRentalType) {
        return CDataManagerDimensionActivationData(G_CDataManager())
            ->reward_data.getRewardSelectItemList(package_key);
    }
    return 0;
}

// IDA_EA: 0x8271338
// IDA_NAME: _ZN7pc_room16GetRewardSectionEii
// DEMANGLED: pc_room::GetRewardSection(int,int)
EventRewardSection* GetRewardSection(int pvp_grade, int rental_type) {
    if (rental_type == kPcRoomRentalType) {
        return CDataManagerPcBangItemRentarData(G_CDataManager())
            ->getSectionData(pvp_grade);
    }
    if (rental_type == kDimensionRentalType) {
        return CDataManagerDimensionActivationData(G_CDataManager())
            ->reward_data.getSectionData(pvp_grade);
    }
    return 0;
}

// IDA_EA: 0x8271396
// IDA_NAME: _ZN7pc_room15GetRentItemInfoER5CUserccR19EventRewardItemInfoi
// DEMANGLED: pc_room::GetRentItemInfo(CUser &,char,char,EventRewardItemInfo &,int)
int GetRentItemInfo(CUser& user,
                    char section_index,
                    char package_index,
                    EventRewardItemInfo& item_info,
                    int rental_type) {
    int selection_index = 0;
    if (GetSelectionIndex(user, selection_index, rental_type) != 1) {
        return 0;
    }
    char character_job = 0;
    if (GetCharacterJob(user, character_job) != 1) {
        return 0;
    }
    std::vector<std::vector<EventRewardItemInfo> >* const selections =
        GetRewardItemSelectionList(
            makeFairPVPEquipmentPackageKey(character_job, selection_index),
            rental_type);
    if (!selections || section_index < 0 ||
        static_cast<size_t>(section_index) >= selections->size()) {
        return 0;
    }
    const std::vector<EventRewardItemInfo>& package =
        selections->at(static_cast<size_t>(section_index));
    if (package_index < 0 ||
        static_cast<size_t>(package_index) >= package.size()) {
        return 0;
    }
    item_info = package[static_cast<size_t>(package_index)];
    return 1;
}

// IDA_EA: 0x82714C5
// IDA_NAME: _ZN7pc_room16ProcessEquipItemER5CUserRKN20InstanceRentalSystem9RentInput10RentInElemERK19EventRewardItemInfoRNS2_10RentOutputEi
// DEMANGLED: pc_room::ProcessEquipItem(CUser &,InstanceRentalSystem::RentInput::RentInElem const&,EventRewardItemInfo const&,InstanceRentalSystem::RentOutput &,int)
int ProcessEquipItem(
    CUser& user,
    const InstanceRentalSystem::RentInput::RentInElem& input,
    const EventRewardItemInfo& item_info,
    InstanceRentalSystem::RentOutput& output,
    int reason) {
    CItem* const item = G_CDataManager()->find_item(item_info.first_dword);
    if (!item) {
        return 1;
    }
    Inven_Item inventory_item;
    item->vtable_->make_item(item, inventory_item);
    if (input.expire_time != -1) {
        inventory_item.set_add_info(input.expire_time);
    }
    inventory_item.SetUpgrade(static_cast<uint8_t>(item_info.word_08));
    const int slot = user.getCurCharacInvenW()->insertItemIntoEquipment(
        inventory_item, ItemAddReason(reason), true, false);
    InstanceRentalSystem::RentalInfo rental_info;
    FillRentalInfo(rental_info, input, item_info, slot,
                   inventory_item.get_add_info(), true);
    output.rental_infos.push_back(rental_info);
    return 0;
}

// IDA_EA: 0x8271666
// IDA_NAME: _ZN7pc_room17ProcessInsertItemER5CUserRKN20InstanceRentalSystem9RentInput10RentInElemERK19EventRewardItemInfoRNS2_10RentOutputEi
// DEMANGLED: pc_room::ProcessInsertItem(CUser &,InstanceRentalSystem::RentInput::RentInElem const&,EventRewardItemInfo const&,InstanceRentalSystem::RentOutput &,int)
int ProcessInsertItem(
    CUser& user,
    const InstanceRentalSystem::RentInput::RentInElem& input,
    const EventRewardItemInfo& item_info,
    InstanceRentalSystem::RentOutput& output,
    int reason) {
    CItem* const item = G_CDataManager()->find_item(item_info.first_dword);
    if (!item) {
        return 1;
    }
    Inven_Item inventory_item;
    item->vtable_->make_item(item, inventory_item);
    inventory_item.SetUpgrade(static_cast<uint8_t>(item_info.word_08));
    if (input.expire_time != -1) {
        inventory_item.set_add_info(input.expire_time);
    }
    const int slot = user.getCurCharacInvenW()->insertItemIntoInventory(
        inventory_item, ItemAddReason(reason), true, false);
    InstanceRentalSystem::RentalInfo rental_info;
    FillRentalInfo(rental_info, input, item_info, slot,
                   inventory_item.get_add_info(), false);
    output.rental_infos.push_back(rental_info);
    return 0;
}

// IDA_EA: 0x8271807
// IDA_NAME: _ZN7pc_room42DeleteAllRentalItemInSameSectionForNewRentER5CUserRSt6vectorIN20InstanceRentalSystem15DeletedRentItemESaIS4_EEi
// DEMANGLED: pc_room::DeleteAllRentalItemInSameSectionForNewRent(CUser &,std::vector<InstanceRentalSystem::DeletedRentItem,std::allocator<InstanceRentalSystem::DeletedRentItem>> &,int)
int DeleteAllRentalItemInSameSectionForNewRent(
    CUser& user,
    std::vector<InstanceRentalSystem::DeletedRentItem>& deleted_items,
    int rental_type) {
    CUserRentalInfoList rental_infos;
    if (user.GetCurrentCharacRentalInfo(rental_infos, rental_type) != 1) {
        return 0;
    }
    CInventory* const inventory_read = user.getCurCharacInvenR();
    CInventory* const inventory_write = user.getCurCharacInvenW();
    for (CUserRentalInfoList::const_iterator it = rental_infos.begin();
         it != rental_infos.end(); ++it) {
        int slot = 0;
        bool equipment = false;
        if (inventory_read->CheckExistItemInInventoryOrEquipment(
                static_cast<int>(it->item_id), slot, equipment) != 1) {
            continue;
        }
        inventory_write->ResetSlot(equipment ? 0 : 1, slot);
        InstanceRentalSystem::DeletedRentItem deleted;
        deleted.is_equipment = equipment;
        deleted.slot = static_cast<uint16_t>(slot);
        deleted.item_id = it->item_id;
        deleted_items.push_back(deleted);
    }
    return 0;
}

}  // namespace pc_room
