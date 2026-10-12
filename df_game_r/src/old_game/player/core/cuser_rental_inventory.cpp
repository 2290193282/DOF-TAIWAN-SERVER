#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include "old_data/db/inventory/db_save_inven.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "infra/log/format/trace.h"

#include <vector>

// IDA_EA: 0x8676DBA
// IDA_NAME: _ZN5CUser21GetInstanceRentalInfoERSt3mapIjSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS3_EESt4lessIjESaISt4pairIKjS5_EEEi
// DEMANGLED: CUser::GetInstanceRentalInfo(std::map<unsigned int,std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>>,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>>>>> &,int)
char CUser::GetInstanceRentalInfo(CUserRentalInfoByCharac& rental_infos,
                                  int rental_type) {
    CUserRentalInfoStore& store = rental_info_store_;
    CUserRentalInfoStore::iterator it = store.find(
        static_cast<unsigned int>(rental_type));
    if (it == store.end()) {
        return 0;
    }

    rental_infos = it->second;
    return static_cast<char>(reinterpret_cast<uintptr_t>(&rental_infos));
}

// IDA_EA: 0x8676E3C
// IDA_NAME: _ZN5CUser29UpdateCurrentCharacRentalInfoERKN20InstanceRentalSystem10RentOutputEib
// DEMANGLED: CUser::UpdateCurrentCharacRentalInfo(InstanceRentalSystem::RentOutput const&,int,bool)
int CUser::UpdateCurrentCharacRentalInfo(
    const InstanceRentalSystem::RentOutput& output,
    int rental_type,
    bool clear_current) {
    const _Charac_info* const current_charac = getCurCharacR();
    const unsigned int charac_no = current_charac
        ? static_cast<unsigned int>(current_charac->fields_.character_no_)
        : 0u;
    int result = static_cast<int>(charac_no);
    if (charac_no) {
        CUserRentalInfoList& rental_infos =
            rental_info_store_[static_cast<unsigned int>(rental_type)][charac_no];
        if (clear_current) {
            rental_infos.clear();
        }

        for (std::vector<InstanceRentalSystem::DeletedRentItem>::const_iterator
                 deleted = output.deleted_items.begin();
             deleted != output.deleted_items.end();
             ++deleted) {
            for (CUserRentalInfoList::iterator info = rental_infos.begin();
                 info != rental_infos.end();
                 ++info) {
                if (info->item_id == deleted->item_id) {
                    rental_infos.erase(info);
                    break;
                }
            }
        }

        for (CUserRentalInfoList::const_iterator info =
                 output.rental_infos.begin();
             info != output.rental_infos.end();
             ++info) {
            rental_infos.push_back(*info);
        }
        result = 0;
    }
    return result;
}

// IDA_EA: 0x8677022
// IDA_NAME: _ZN5CUser20DeleteRentalItemInfoEi
// DEMANGLED: CUser::DeleteRentalItemInfo(int)
void CUser::DeleteRentalItemInfo(int rental_type) {
    rental_info_store_[static_cast<unsigned int>(rental_type)].clear();
}

// IDA_EA: 0x8677050
// IDA_NAME: _ZN5CUser33DeleteCurrentCharacRentalItemInfoEi
// DEMANGLED: CUser::DeleteCurrentCharacRentalItemInfo(int)
int CUser::DeleteCurrentCharacRentalItemInfo(int rental_type) {
    const _Charac_info* const current_charac = getCurCharacR();
    if (!current_charac || !current_charac->fields_.character_no_) {
        return 0;
    }
    const unsigned int charac_no = static_cast<unsigned int>(
        current_charac->fields_.character_no_);

    CUserRentalInfoStore::iterator type_it = rental_info_store_.find(
        static_cast<unsigned int>(rental_type));
    if (type_it == rental_info_store_.end()) {
        return 0;
    }
    return static_cast<int>(type_it->second.erase(charac_no));
}

// IDA_EA: 0x8677166
// IDA_NAME: _ZN5CUser26GetCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
// DEMANGLED: CUser::GetCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
int CUser::GetCurrentCharacRentalInfo(CUserRentalInfoList& rental_infos,
                                      int rental_type) {
    return getCurrentCharacRentalInfo(rental_infos, rental_type);
}

// IDA_EA: 0x8677188
// IDA_NAME: _ZN5CUser15checkRentedItemEjii
// DEMANGLED: CUser::checkRentedItem(unsigned int,int,int)
int CUser::checkRentedItem(unsigned int charac_no,
                           int item_id,
                           int rental_type) {
    CUserRentalInfoStore::iterator type_it = rental_info_store_.find(
        static_cast<unsigned int>(rental_type));
    if (type_it == rental_info_store_.end()) {
        return 0;
    }
    CUserRentalInfoByCharac::iterator charac_it = type_it->second.find(
        charac_no);
    if (charac_it == type_it->second.end()) {
        return 0;
    }
    for (CUserRentalInfoList::const_iterator info = charac_it->second.begin();
         info != charac_it->second.end();
         ++info) {
        if (info->item_id == static_cast<unsigned int>(item_id)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x86772D2
// IDA_NAME: _ZNK5CUser22HasCurCharacRentalItemEi
// DEMANGLED: CUser::HasCurCharacRentalItem(int)const
int CUser::HasCurCharacRentalItem(int rental_type) const {
    const _Charac_info* const current_charac = getCurCharacR();
    if (!current_charac || !current_charac->fields_.character_no_) {
        return 0;
    }
    const unsigned int charac_no = static_cast<unsigned int>(
        current_charac->fields_.character_no_);

    const CUserRentalInfoStore::const_iterator type_it =
        rental_info_store_.find(static_cast<unsigned int>(rental_type));
    if (type_it == rental_info_store_.end()) {
        return 0;
    }
    const CUserRentalInfoByCharac::const_iterator charac_it =
        type_it->second.find(charac_no);
    return charac_it != type_it->second.end() && !charac_it->second.empty();
}

// IDA_EA: 0x86773EA
// IDA_NAME: _ZN5CUser26getCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
// DEMANGLED: CUser::getCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
int CUser::getCurrentCharacRentalInfo(CUserRentalInfoList& rental_infos,
                                      int rental_type) {
    const _Charac_info* const current_charac = getCurCharacR();
    if (!current_charac || !current_charac->fields_.character_no_) {
        return 0;
    }
    const unsigned int charac_no = static_cast<unsigned int>(
        current_charac->fields_.character_no_);

    CUserRentalInfoStore::const_iterator type_it =
        rental_info_store_.find(static_cast<unsigned int>(rental_type));
    if (type_it == rental_info_store_.end()) {
        return 0;
    }
    CUserRentalInfoByCharac::const_iterator charac_it =
        type_it->second.find(charac_no);
    if (charac_it == type_it->second.end()) {
        return 0;
    }
    rental_infos = charac_it->second;
    return 1;
}

// IDA_EA: 0x8677516
// IDA_NAME: _ZN5CUser26setCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
// DEMANGLED: CUser::setCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
CUserRentalInfoList* CUser::setCurrentCharacRentalInfo(
    CUserRentalInfoList& rental_infos,
    int rental_type) {
    const _Charac_info* const current_charac = getCurCharacR();
    if (!current_charac || !current_charac->fields_.character_no_) {
        return 0;
    }
    const unsigned int charac_no = static_cast<unsigned int>(
        current_charac->fields_.character_no_);

    CUserRentalInfoList& stored =
        rental_info_store_[static_cast<unsigned int>(rental_type)][charac_no];
    stored = rental_infos;
    return &stored;
}

// IDA_EA: 0x8677584
// IDA_NAME: _ZN5CUser32removeRentalItemInfoFromInvenBufEP18SIG_SAVE_INVENTORYi
// DEMANGLED: CUser::removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY *,int)
int CUser::removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY* payload,
                                            int rental_type) {
    if (!payload) {
        return 0;
    }

    std::vector<InstanceRentalSystem::RentalInfo> rental_infos;
    if (getCurrentCharacRentalInfo(rental_infos, rental_type) != 1) {
        return 0;
    }

    for (std::vector<InstanceRentalSystem::RentalInfo>::iterator it =
             rental_infos.begin();
         it != rental_infos.end();
         ++it) {
        int slot = 0;
        bool equipment = false;
        CInventory* const inventory = static_cast<CInventory*>(
            this->getCurCharacInvenR());
        if (inventory->CheckExistItemInInventoryOrEquipment(
                static_cast<int>(it->item_id), slot, equipment) == 1) {
            it->equip_flag = equipment;
            if (equipment) {
                // SIG_SAVE_INVENTORY now carries the complete equipment
                // prefix (avatar slots 0..10 followed by ordinary slots
                // 11..22).  The old frame started ordinary equipment at
                // index zero, so subtracting 11 here clears an avatar record
                // and leaves the rented ordinary item persisted.
                if (slot < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
                    slot > oldsrv::config::kOrdinaryEquipmentTypeLast) {
                    LogManager::logFormat(
                        1,
                        "user.cpp",
                        "bool CUser::removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY*, int)",
                        0x6D15,
                        "Rent equipment slot out of save range. Acc(%d), itemIndex(%d), slot(%d)",
                        this->get_acc_id(),
                        static_cast<int>(it->item_id),
                        slot);
                    inventory->ShowAllExistItemsForErrorCheck();
                    continue;
                }
                payload->equipment_items[slot].reset();
            } else {
                payload->stackable_items[slot].reset();
            }
        } else {
            LogManager::logFormat(
                1,
                "user.cpp",
                "bool CUser::removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY*, int)",
                0x6D15,
                "Rent error. CUser::removeRentalItemInfoFromInvenBuf return false. Acc(%d), itemIndex(%d)",
                this->get_acc_id(),
                static_cast<int>(it->item_id));
            inventory->ShowAllExistItemsForErrorCheck();
        }
    }

    setCurrentCharacRentalInfo(rental_infos, rental_type);
    return 1;
}

// IDA_EA: 0x867776C
// IDA_NAME: _ZN5CUser36removeRentalItemInfoFromInvenLoadBufEP18SIG_LOAD_INVENTORYi
// DEMANGLED: CUser::removeRentalItemInfoFromInvenLoadBuf(SIG_LOAD_INVENTORY *,int)
int CUser::removeRentalItemInfoFromInvenLoadBuf(SIG_LOAD_INVENTORY* payload,
                                                int rental_type) {
    if (!payload) {
        return 0;
    }

    std::vector<InstanceRentalSystem::RentalInfo> rental_infos;
    if (getCurrentCharacRentalInfo(rental_infos, rental_type) != 1) {
        return 0;
    }

    for (std::vector<InstanceRentalSystem::RentalInfo>::iterator it =
             rental_infos.begin();
         it != rental_infos.end();
         ++it) {
        int slot = 0;
        bool equipment = false;
        CInventory* const inventory = static_cast<CInventory*>(
            this->getCurCharacInvenR());
        if (inventory->CheckExistItemInInventoryOrEquipment(
                static_cast<int>(it->item_id), slot, equipment) == 1) {
            it->equip_flag = equipment;
            if (equipment) {
                payload->equipment[
                    slot - oldsrv::config::kOrdinaryEquipmentTypeFirst]
                    .reset();
            } else {
                payload->inventory[slot].reset();
            }
        } else {
            LogManager::logFormat(
                1,
                "user.cpp",
                "bool CUser::removeRentalItemInfoFromInvenLoadBuf(SIG_LOAD_INVENTORY*, int)",
                0x6D54,
                "Rent error. CUser::removeRentalItemInfoFromInvenLoadBuf return false. Acc(%d), itemIndex(%d)",
                this->get_acc_id(),
                static_cast<int>(it->item_id));
            inventory->ShowAllExistItemsForErrorCheck();
        }
    }

    setCurrentCharacRentalInfo(rental_infos, rental_type);
    return 1;
}
