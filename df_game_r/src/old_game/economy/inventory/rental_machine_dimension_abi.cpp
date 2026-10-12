#include "old_game/economy/inventory/rental_machine_dimension_abi.h"

#include "core/config/env/environment.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_game/dungeon/room/pc_room_public_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <cstdio>

// IDA_EA: 0x826FA6C
// IDA_NAME: _ZN22RentalMachineDimension26RentalMachineDimensionImplC2Ev
RentalMachineDimension::RentalMachineDimensionImpl::
    RentalMachineDimensionImpl()
    : log_file_name(), rentable_item_ids() {}

// IDA_EA: 0x826FB0C
// IDA_NAME: _ZN22RentalMachineDimension26RentalMachineDimensionImplD2Ev
RentalMachineDimension::RentalMachineDimensionImpl::
    ~RentalMachineDimensionImpl() {}

// IDA_EA: 0x826EAFC
// IDA_NAME: _ZN22RentalMachineDimensionC2Ev
RentalMachineDimension::RentalMachineDimension()
    : IRentalMachine(), impl_(new RentalMachineDimensionImpl()) {
    initLoggingEnvironment();
}

// IDA_EA: 0x826EBA2
// IDA_NAME: _ZN22RentalMachineDimensionD2Ev
RentalMachineDimension::~RentalMachineDimension() {}

// IDA_EA: 0x826EC22
// IDA_NAME: _ZN22RentalMachineDimension4RentER5CUserRKN20InstanceRentalSystem9RentInputERNS2_10RentOutputE
int RentalMachineDimension::Rent(
    CUser& user,
    const InstanceRentalSystem::RentInput& input,
    InstanceRentalSystem::RentOutput& output) {
    if (pc_room::CheckRentableEnvironmentAtChannelType(user) != 1) {
        return 7;
    }
    if (checkRentalLevel(user.get_charac_level()) != 1) {
        return 14;
    }
    if (checkRentalGrowJob(user.getCurCharFirstGrowType()) != 1) {
        return 254;
    }

    const int check_result = CheckupAllRentalItems(user, input);
    if (check_result != 0) {
        return check_result;
    }

    pc_room::DeleteAllRentalItemInSameSectionForNewRent(
        user, output.deleted_items, 2);
    for (std::vector<InstanceRentalSystem::RentInput::RentInElem>::const_iterator
             it = input.entries.begin();
         it != input.entries.end(); ++it) {
        EventRewardItemInfo item_info;
        if (GetRentItemInfo(user, *it, item_info) != 1) {
            LogManager::logFormat(
                1, "RentalMachineDimension.cpp",
                "virtual ENUM_ERROR RentalMachineDimension::Rent(CUser&, const InstanceRentalSystem::RentInput&, InstanceRentalSystem::RentOutput&)",
                67,
                "Rent error. GetDimensionRentItemInfo return false. scriptSectionIndex(%d), scriptSeqIndex(%d)",
                static_cast<int>(it->section_index),
                static_cast<int>(it->package_index));
            return 1;
        }

        if (it->equip_flag) {
            pc_room::ProcessEquipItem(user, *it, item_info, output, 62);
        } else {
            pc_room::ProcessInsertItem(user, *it, item_info, output, 62);
        }
        if (input.write_log) {
            log(user, static_cast<int>(item_info.first_dword));
        }
    }
    return 0;
}

// IDA_EA: 0x826EE94
// IDA_NAME: _ZN22RentalMachineDimension5ReadyEv
int RentalMachineDimension::Ready() { return checkAllRentalItemScript(); }

// IDA_EA: 0x826EEA8
// IDA_NAME: _ZN22RentalMachineDimension17CheckRentableItemEj
int RentalMachineDimension::CheckRentableItem(unsigned int item_id) {
    return impl_->rentable_item_ids.find(item_id) !=
           impl_->rentable_item_ids.end();
}

// IDA_EA: 0x826EF16
// IDA_NAME: _ZN22RentalMachineDimension3logER5CUseri
int RentalMachineDimension::log(CUser& user, int item_id) {
    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user.GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    CMyFileLog file_log("void RentalMachineDimension::log(CUser&, int)",
                        113);
    return file_log(impl_->log_file_name,
                    "[RENT] Dimension m_id(%d) char_no(%d) server_info(%d) item_id(%d)",
                    user.get_acc_id(), user.get_charac_no(-1), server_group,
                    item_id);
}

// IDA_EA: 0x826EFDC
// IDA_NAME: _ZN22RentalMachineDimension22initLoggingEnvironmentEv
int RentalMachineDimension::initLoggingEnvironment() {
    return std::snprintf(impl_->log_file_name, 0x103u,
                         "./log/%s/dw2_RentalItems",
                         G_CEnvironment()->get_file_name());
}

// IDA_EA: 0x826F022
// IDA_NAME: _ZN22RentalMachineDimension24checkAllRentalItemScriptEv
int RentalMachineDimension::checkAllRentalItemScript() {
    CDataManager* const data_manager = G_CDataManager();
    DimensionActivationData* const activation =
        CDataManagerDimensionActivationData(data_manager);
    for (std::map<int, std::vector<std::vector<EventRewardItemInfo> > >::
             const_iterator it = activation->reward_data.select_groups.begin();
         it != activation->reward_data.select_groups.end(); ++it) {
        if (checkSelectionListScript(it->second) != 1) {
            return 0;
        }
    }
    for (std::map<std::pair<int, int>,
                  std::vector<std::vector<EventRewardItemInfo> > >::
             const_iterator it = activation->reward_data.give_groups.begin();
         it != activation->reward_data.give_groups.end(); ++it) {
        if (checkSelectionListScript(it->second) != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x826F14E
// IDA_NAME: _ZN22RentalMachineDimension24checkSelectionListScriptERKSt6vectorIS0_I19EventRewardItemInfoSaIS1_EESaIS3_EE
int RentalMachineDimension::checkSelectionListScript(
    const std::vector<std::vector<EventRewardItemInfo> >& selections) {
    for (std::vector<std::vector<EventRewardItemInfo> >::const_iterator group =
             selections.begin();
         group != selections.end(); ++group) {
        for (std::vector<EventRewardItemInfo>::const_iterator item_info =
                 group->begin();
             item_info != group->end(); ++item_info) {
            CItem* const item =
                G_CDataManager()->find_item(item_info->first_dword);
            if (!item || item->IsImpossibleAmplify() != 1 ||
                item->IsImpossibleCharacCargo() != 1 ||
                item->IsImpossibleDisjoint() != 1 ||
                item->IsImpossibleGift() != 1 ||
                item->IsImpossibleUpgrade() != 1 ||
                item->GetAttachType() != 2) {
                return 0;
            }
            impl_->rentable_item_ids.insert(item_info->first_dword);
        }
    }
    return 1;
}

// IDA_EA: 0x826F52C
// IDA_NAME: _ZN22RentalMachineDimension21CheckupAllRentalItemsER5CUserRKN20InstanceRentalSystem9RentInputE
int RentalMachineDimension::CheckupAllRentalItems(
    CUser& user,
    const InstanceRentalSystem::RentInput& input) {
    int inventory_item_count = 0;
    for (std::vector<InstanceRentalSystem::RentInput::RentInElem>::const_iterator
             it = input.entries.begin();
         it != input.entries.end(); ++it) {
        EventRewardItemInfo item_info;
        if (GetRentItemInfo(user, *it, item_info) != 1) {
            return 1;
        }
        CItem* const item = G_CDataManager()->find_item(item_info.first_dword);
        if (!item) {
            return 1;
        }
        Inven_Item inventory_item;
        item->vtable_->make_item(item, inventory_item);
        if (it->equip_flag) {
            if (user.getCurCharacInvenR()->tryInsertItemIntoEquipment(
                    inventory_item) < 0) {
                return 203;
            }
        } else {
            if (user.getCurCharacInvenR()->tryInsertItemIntoInventory(
                    inventory_item) < 0) {
                return 203;
            }
            ++inventory_item_count;
        }
    }
    return pc_room::CheckInventoryEquipmentEmptyCount(user,
                                                        inventory_item_count) ==
                   1
               ? 0
               : 203;
}

// IDA_EA: 0x826F7E4
int RentalMachineDimension::checkRentalLevel(int charac_level) {
    return charac_level > 64;
}

// IDA_EA: 0x826F7FC
int RentalMachineDimension::checkRentalGrowJob(int first_grow_type) {
    return first_grow_type != 0;
}

// IDA_EA: 0x826F814
std::vector<std::vector<EventRewardItemInfo> >*
RentalMachineDimension::GetRewardItemGiveList(int charac_job,
                                               int first_grow_type) {
    return CDataManagerDimensionActivationData(G_CDataManager())
        ->reward_data.getRewardGiveItemList(charac_job, first_grow_type);
}

// IDA_EA: 0x826F842
int RentalMachineDimension::GetRentItemInfo(
    CUser& user,
    const InstanceRentalSystem::RentInput::RentInElem& input,
    EventRewardItemInfo& item_info) {
    int selection_index = 0;
    if (pc_room::GetSelectionIndex(user, selection_index, 2) != 1) {
        return 0;
    }
    char character_job = 0;
    if (pc_room::GetCharacterJob(user, character_job) != 1) {
        return 0;
    }

    std::vector<std::vector<EventRewardItemInfo> >* item_lists = 0;
    if (input.rent_type == 1) {
        item_lists = GetRewardItemGiveList(
            character_job, user.getCurCharFirstGrowType());
    } else {
        item_lists = pc_room::GetRewardItemSelectionList(
            makeFairPVPEquipmentPackageKey(character_job, selection_index), 2);
    }
    if (!item_lists || input.section_index >= item_lists->size()) {
        return 0;
    }
    const std::vector<EventRewardItemInfo>& package =
        item_lists->at(input.section_index);
    if (input.package_index >= package.size()) {
        return 0;
    }
    item_info = package[input.package_index];
    return 1;
}
