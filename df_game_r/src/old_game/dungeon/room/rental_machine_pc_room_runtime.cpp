#include "old_game/dungeon/room/rental_machine_pc_room_runtime.h"

#include "core/config/env/environment.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/room/pc_room_public_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <cstdio>
#include <iostream>

// IDA_EA: 0x8271A80
// IDA_NAME: _ZN19RentalMachinePcRoom23RentalMachinePcRoomImplC2Ev
RentalMachinePcRoom::RentalMachinePcRoomImpl::RentalMachinePcRoomImpl()
    : log_file_name(), rentable_item_ids() {}

// IDA_EA: 0x8271D50
// IDA_NAME: _ZN19RentalMachinePcRoom23RentalMachinePcRoomImplD2Ev
RentalMachinePcRoom::RentalMachinePcRoomImpl::~RentalMachinePcRoomImpl() {}

// IDA_EA: 0x82703C0
// IDA_NAME: _ZN19RentalMachinePcRoomC2Ev
RentalMachinePcRoom::RentalMachinePcRoom()
    : IRentalMachine(), impl_(new RentalMachinePcRoomImpl()) {
    initLoggingEnvironment();
}

// IDA_EA: 0x8270466
// IDA_NAME: _ZN19RentalMachinePcRoomD2Ev
RentalMachinePcRoom::~RentalMachinePcRoom() {}

// IDA_EA: 0x82704E6
// IDA_NAME: _ZN19RentalMachinePcRoom4RentER5CUserRKN20InstanceRentalSystem9RentInputERNS2_10RentOutputE
int RentalMachinePcRoom::Rent(
    CUser& user,
    const InstanceRentalSystem::RentInput& input,
    InstanceRentalSystem::RentOutput& output) {
    if (pc_room::IsPcRoomUser(user) != 1 && input.force_pcroom != 1) {
        return 8;
    }
    if (pc_room::CheckRentableEnvironment(user) != 1) {
        return 7;
    }

    const int check_result = CheckupAllRentalItems(user, input);
    if (check_result != 0) {
        return check_result;
    }

    pc_room::DeleteAllRentalItemInSameSectionForNewRent(
        user, output.deleted_items, 1);
    for (std::vector<InstanceRentalSystem::RentInput::RentInElem>::
             const_iterator it = input.entries.begin();
         it != input.entries.end(); ++it) {
        EventRewardItemInfo item_info;
        if (pc_room::GetRentItemInfo(user, static_cast<char>(it->section_index),
                                     static_cast<char>(it->package_index),
                                     item_info, 1) != 1) {
            LogManager::logFormat(
                1, "RentalSystem.cpp",
                "virtual ENUM_ERROR RentalMachinePcRoom::Rent(CUser&, const InstanceRentalSystem::RentInput&, InstanceRentalSystem::RentOutput&)",
                248,
                "Rent error. GetRentItemInfo return false. scriptSectionIndex(%d), scriptSeqIndex(%d)",
                static_cast<int>(it->section_index),
                static_cast<int>(it->package_index));
            return 1;
        }

        if (it->equip_flag) {
            pc_room::ProcessEquipItem(user, *it, item_info, output, 61);
        } else {
            pc_room::ProcessInsertItem(user, *it, item_info, output, 61);
        }
        if (input.write_log) {
            log(user, static_cast<int>(item_info.first_dword));
        }
    }
    return 0;
}

// IDA_EA: 0x827073E
// IDA_NAME: _ZN19RentalMachinePcRoom5ReadyEv
int RentalMachinePcRoom::Ready() { return checkAllRentalItemScript(); }

// IDA_EA: 0x8270752
// IDA_NAME: _ZN19RentalMachinePcRoom17CheckRentableItemEj
int RentalMachinePcRoom::CheckRentableItem(unsigned int item_id) {
    return impl_->rentable_item_ids.find(item_id) !=
           impl_->rentable_item_ids.end();
}

// IDA_EA: 0x82707C0
// IDA_NAME: _ZN19RentalMachinePcRoom3logER5CUseri
int RentalMachinePcRoom::log(CUser& user, int item_id) {
    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user.GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    CMyFileLog file_log("void RentalMachinePcRoom::log(CUser&, int)", 302);
    return file_log(impl_->log_file_name,
                    "[RENT] m_id(%d) char_no(%d) server_info(%d) item_id(%d)",
                    user.get_acc_id(), user.get_charac_no(-1), server_group,
                    item_id);
}

// IDA_EA: 0x8270886
// IDA_NAME: _ZN19RentalMachinePcRoom22initLoggingEnvironmentEv
int RentalMachinePcRoom::initLoggingEnvironment() {
    return std::snprintf(impl_->log_file_name, 0x103u,
                         "./log/%s/dw2_RentalItems",
                         G_CEnvironment()->get_file_name());
}

// IDA_EA: 0x82708CC
// IDA_NAME: _ZN19RentalMachinePcRoom24checkAllRentalItemScriptEv
int RentalMachinePcRoom::checkAllRentalItemScript() {
    PcBangItemRentarData* const rental_data =
        CDataManagerPcBangItemRentarData(G_CDataManager());
    for (std::map<int, std::vector<std::vector<EventRewardItemInfo> > >::
             const_iterator group = rental_data->selection_groups.begin();
         group != rental_data->selection_groups.end(); ++group) {
        for (std::vector<std::vector<EventRewardItemInfo> >::const_iterator
                 selection = group->second.begin();
             selection != group->second.end(); ++selection) {
            for (std::vector<EventRewardItemInfo>::const_iterator item_info =
                     selection->begin();
                 item_info != selection->end(); ++item_info) {
                CItem* const item =
                    G_CDataManager()->find_item(item_info->first_dword);
                if (!item) {
                    std::cerr << "[RentalSystem::Ready()] Can`t find item["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->IsImpossibleAmplify() != 1) {
                    std::cerr << "[RentalSystem::Ready()] Impossible contents(amplify upgrade) is not checked. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->IsImpossibleCharacCargo() != 1) {
                    std::cerr << "[RentalSystem::Ready()] Impossible contents(charac cargo) is not checked. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->IsImpossibleDisjoint() != 1) {
                    std::cerr << "[RentalSystem::Ready()] Impossible contents(disjoint) is not checked. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->IsImpossibleGift() != 1) {
                    std::cerr << "[RentalSystem::Ready()] Impossible contents(gift) is not checked. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->IsImpossibleUpgrade() != 1) {
                    std::cerr << "[RentalSystem::Ready()] Impossible contents(upgrade) is not checked. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                if (item->GetAttachType() != 2) {
                    std::cerr << "[RentalSystem::Ready()] AtttachType is not trade delete. ["
                              << item_info->first_dword << "]" << std::endl;
                    return 0;
                }
                impl_->rentable_item_ids.insert(item_info->first_dword);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8270D1E
// IDA_NAME: _ZN19RentalMachinePcRoom21CheckupAllRentalItemsER5CUserRKN20InstanceRentalSystem9RentInputE
int RentalMachinePcRoom::CheckupAllRentalItems(
    CUser& user,
    const InstanceRentalSystem::RentInput& input) {
    int inventory_item_count = 0;
    for (std::vector<InstanceRentalSystem::RentInput::RentInElem>::
             const_iterator it = input.entries.begin();
         it != input.entries.end(); ++it) {
        EventRewardItemInfo item_info;
        if (pc_room::GetRentItemInfo(user, static_cast<char>(it->section_index),
                                     static_cast<char>(it->package_index),
                                     item_info, 1) != 1) {
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
    return pc_room::CheckInventoryEmptyCount(user, inventory_item_count) == 1
               ? 0
               : 203;
}
