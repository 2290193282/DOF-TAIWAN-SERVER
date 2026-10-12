#include "old_game/appearance/avatar/compound_avatar.h"

#include <cstdio>
#include <ctime>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/avatar_aura/db_update_avatar_history.h"
#include "old_game/appearance/avatar/compound_avatar_messages.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"
#include "common/manager/system_time.h"

namespace {

typedef int (*AvatarFunctionSlot)(const CItem*, int);

int GetItemType(const CItem* item) {
    return item->vtable_->get_item_type(item);
}

bool IsAvatarCompoundEnabled(const CItem* item) {
    const AvatarFunctionSlot enabled =
        reinterpret_cast<AvatarFunctionSlot>(item->vtable_->slots_6_through_9[3]);
    return enabled && enabled(item, 1) == 1;
}

bool ContainsItem(const CompoundAvatarItemList& items, int item_id) {
    for (CompoundAvatarItemList::const_iterator item = items.begin();
         item != items.end();
         ++item) {
        if (item->first == item_id) {
            return true;
        }
    }
    return false;
}

unsigned int AncientRareCount(const STAncientRareAvatarCube& cube,
                              int equipment_type) {
    switch (equipment_type) {
    case 0: return cube.grade0_count;
    case 1: return cube.grade1_count;
    case 2: return cube.grade2_count;
    case 3: return cube.grade3_count;
    case 4: return cube.grade4_count;
    case 5: return cube.grade5_count;
    case 6: return cube.grade6_count;
    case 7: return cube.grade7_count;
    case 8: return cube.grade8_count;
    case 9: return cube.grade9_count;
    default: return 0;
    }
}

const CompoundAvatarItemList& AncientRareItems(
    const STAncientRareAvatarCube& cube,
    int equipment_type) {
    static const CompoundAvatarItemList kEmptyItems;
    switch (equipment_type) {
    case 0: return cube.grade0_items;
    case 1: return cube.grade1_items;
    case 2: return cube.grade2_items;
    case 3: return cube.grade3_items;
    case 4: return cube.grade4_items;
    case 5: return cube.grade5_items;
    case 6: return cube.grade6_items;
    case 7: return cube.grade7_items;
    case 8: return cube.grade8_items;
    case 9: return cube.grade9_items;
    default: return kEmptyItems;
    }
}

int FindCompoundGrade(const tagCompoundAvatarTable& table,
                      uint8_t compound_kind,
                      int recipe_item_id) {
    for (size_t grade = 1; grade < table.entries.size(); ++grade) {
        const tagCompoundAvatar* const info = table.getCompoundAvatarInfo(
            static_cast<int>(grade));
        if (!info) {
            continue;
        }

        if (compound_kind == 1 &&
            ContainsItem(info->material_upper_items, recipe_item_id)) {
            return static_cast<int>(grade);
        }
        if (compound_kind == 3 &&
            ContainsItem(info->material_lower_items, recipe_item_id)) {
            return static_cast<int>(grade);
        }
        if (compound_kind == 2 &&
            ContainsItem(info->material_master_items, recipe_item_id)) {
            return static_cast<int>(grade);
        }
        if (compound_kind == 4 &&
            ContainsItem(info->ancient_rare_cube.common_items, recipe_item_id)) {
            return static_cast<int>(grade);
        }
        if (compound_kind == 0 &&
            ContainsItem(info->material_items, recipe_item_id)) {
            return static_cast<int>(grade);
        }
    }
    return 0;
}

int DeleteCompoundMaterials(CInventory* inventory,
                            WongWork::CCompoundAvatar::tagCompoundInfo& info,
                            const tagCompoundAvatar& table_info) {
    const CompoundAvatarItemList* materials = 0;
    size_t expected_count = 0;

    switch (info.compound_kind) {
    case 0:
        materials = &table_info.material_items;
        expected_count = table_info.material_items.size();
        break;
    case 1:
        materials = &table_info.material_upper_items;
        expected_count = table_info.material_upper_items.size();
        break;
    case 3:
        materials = &table_info.material_lower_items;
        expected_count = table_info.material_lower_items.size();
        break;
    case 2:
        materials = &table_info.material_master_items;
        expected_count = table_info.material_master_items.size();
        break;
    case 4:
        materials = &table_info.ancient_rare_cube.common_items;
        // The original body compares the ancient-cube deletion result to the
        // lower-material list size, including this historical mismatch.
        expected_count = table_info.material_lower_items.size();
        break;
    default:
        // _MakeCompoundInfo leaves compound_grade zero for these unsupported
        // kinds, so normal callers never reach this branch.
        return 0;
    }

    bool equipment_changed = false;
    const int deleted = inventory->delete_event_items(
        *materials,
        info.avatar_mutations,
        equipment_changed,
        static_cast<eMoneySubReason>(18),
        static_cast<eItemDelReason>(18),
        false);
    return deleted == static_cast<int>(expected_count) ? 0 : 22;
}

int GradeBucket(const CItem* item) {
    return item->get_rarity() == 2 ? 2 : item->get_grade() - 1;
}

}  // namespace

// IDA_EA: 0x8331EFE
// IDA_NAME: _ZNK8WongWork15CCompoundAvatar17_MakeCompoundInfoEPK5CUserPK19MSG_COMPOUND_AVATAR
// DEMANGLED: WongWork::CCompoundAvatar::_MakeCompoundInfo(CUser const*,MSG_COMPOUND_AVATAR const*)const
WongWork::CCompoundAvatar::tagCompoundInfo
WongWork::CCompoundAvatar::_MakeCompoundInfo(
    const CUser* user,
    const MSG_COMPOUND_AVATAR* request) const {
    tagCompoundInfo info;
    info.clear();

    info.upper_compound = request->upper_compound == 1;
    info.requested_result_item = static_cast<int>(request->requested_result_item);
    info.requested_avatar_option = request->requested_avatar_option;
    info.recipe_slot = request->recipe_slot;
    info.recipe = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), static_cast<int>(info.recipe_slot));
    info.first_avatar_slot = request->first_avatar_slot;
    info.first_avatar = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(2), static_cast<int>(info.first_avatar_slot));
    info.second_avatar_slot = request->second_avatar_slot;
    info.second_avatar = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(2), static_cast<int>(info.second_avatar_slot));
    info.first_avatar_prototype = G_CDataManager()->find_item(
        static_cast<int>(request->first_avatar_item_id));
    info.second_avatar_prototype = G_CDataManager()->find_item(
        static_cast<int>(request->second_avatar_item_id));

    if (!info.first_avatar_prototype || !info.second_avatar_prototype ||
        static_cast<uint32_t>(info.first_avatar.item_id_) !=
            request->first_avatar_item_id ||
        static_cast<uint32_t>(info.second_avatar.item_id_) !=
            request->second_avatar_item_id) {
        info.invalid_request = true;
        return info;
    }

    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        const int first_usable = info.first_avatar_prototype->check_job_type(
            static_cast<char>(job));
        if (first_usable ==
                info.second_avatar_prototype->check_job_type(
                    static_cast<char>(job)) &&
            first_usable) {
            info.character_job = job;
            break;
        }
    }

    if (info.character_job != oldsrv::config::kCharacterJobInvalid) {
        info.compound_grade = FindCompoundGrade(
            *table_for_job(info.character_job),
            request->compound_kind,
            info.recipe.item_id_);
        info.compound_kind = request->compound_kind;
    }
    return info;
}

// IDA_EA: 0x833260C
// IDA_NAME: _ZNK8WongWork15CCompoundAvatar15_CheckConditionEPK5CUserRKNS0_15tagCompoundInfoE
// DEMANGLED: WongWork::CCompoundAvatar::_CheckCondition(CUser const*, WongWork::CCompoundAvatar::tagCompoundInfo const&)const
int WongWork::CCompoundAvatar::_CheckCondition(
    const CUser* user,
    const tagCompoundInfo& info) const {
    if (info.compound_kind == 4 && info.recipe.item_id_ != 2660548) {
        return 17;
    }
    if (info.requested_avatar_option > 0xff || info.invalid_request) {
        return 17;
    }
    if (info.character_job == oldsrv::config::kCharacterJobInvalid ||
        info.compound_grade == 0) {
        return 23;
    }
    if (!info.first_avatar.item_id_ || !info.second_avatar.item_id_) {
        return 17;
    }

    const CInventory* const inventory = user->getCurCharacInvenR();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrR();
    const int first_key = info.first_avatar.get_add_info();
    const int second_key = info.second_avatar.get_add_info();
    if (WongWork::CAvatarItemMgr::IsTempKey(first_key) ||
        WongWork::CAvatarItemMgr::IsTempKey(second_key)) {
        return 1;
    }
    if (!info.first_avatar_prototype->vtable_->is_avatar_item(
            info.first_avatar_prototype) ||
        !info.second_avatar_prototype->vtable_->is_avatar_item(
            info.second_avatar_prototype)) {
        return 17;
    }

    const int equipment_type = GetItemType(info.first_avatar_prototype);
    if (equipment_type != GetItemType(info.second_avatar_prototype)) {
        return 23;
    }
    if (!IsAvatarCompoundEnabled(info.first_avatar_prototype) ||
        !IsAvatarCompoundEnabled(info.second_avatar_prototype)) {
        throw static_cast<ENUM_ERROR>(23);
    }

    const tagCompoundAvatar* const table_info = table_for_job(
        info.character_job)->getCompoundAvatarInfo(info.compound_grade);
    if (info.first_avatar_prototype->get_rarity() > 1 ||
        info.second_avatar_prototype->get_rarity() > 1 ||
        info.first_avatar_prototype->get_grade() !=
            info.second_avatar_prototype->get_grade()) {
        return 23;
    }

    if (!info.upper_compound) {
        const int avatar_grade = info.first_avatar_prototype->get_grade();
        const CompoundAvatarItemList* results = 0;
        if (avatar_grade == table_info->grade) {
            results = &table_info->normal_avatar_items[equipment_type];
        } else if (avatar_grade == table_info->upper_grade) {
            results = &table_info->rare_avatar_items[equipment_type];
        } else {
            return 1;
        }
        if (!ContainsItem(*results, info.requested_result_item)) {
            return 1;
        }
    }

    if (info.first_avatar_prototype->get_grade() < info.compound_grade ||
        info.second_avatar_prototype->get_grade() < info.compound_grade ||
        info.first_avatar_prototype->get_grade() > table_info->upper_grade ||
        info.second_avatar_prototype->get_grade() > table_info->upper_grade ||
        !info.first_avatar_prototype->check_job_type(
            static_cast<char>(user->get_charac_job())) ||
        !info.second_avatar_prototype->check_job_type(
            static_cast<char>(user->get_charac_job()))) {
        return 23;
    }

    switch (info.compound_kind) {
    case 0:
        if (inventory->CheckNeedItemList(table_info->material_items) != 1) {
            return 22;
        }
        break;
    case 1:
        if (inventory->CheckNeedItemList(table_info->material_upper_items) != 1) {
            return 22;
        }
        break;
    case 3:
        if (inventory->CheckNeedItemList(table_info->material_lower_items) != 1) {
            return 22;
        }
        break;
    case 2:
        if (inventory->CheckNeedItemList(table_info->material_master_items) != 1) {
            return 22;
        }
        break;
    case 4:
        if (inventory->CheckNeedItemList(
                table_info->ancient_rare_cube.common_items) != 1) {
            return 22;
        }
        break;
    default:
        break;
    }

    const int current_time = GlobalData::s_systemTime_.getCurSec();
    const int first_remain = avatar_manager->GetRemainDate(first_key,
                                                            current_time);
    const int second_remain = avatar_manager->GetRemainDate(second_key,
                                                             current_time);
    const bool first_unlimited = avatar_manager->GetExpireDate(first_key) == 0;
    const bool second_unlimited =
        avatar_manager->GetExpireDate(second_key) == 0;
    if (first_unlimited != second_unlimited) {
        return 19;
    }
    if (!first_unlimited && (first_remain <= 5 || second_remain <= 5)) {
        return 24;
    }
    if (info.first_avatar.upgrade_separate_.IsTradeRestriction() ||
        info.second_avatar.upgrade_separate_.IsTradeRestriction()) {
        return 23;
    }
    return 0;
}

// IDA_EA: 0x8332EE8
// IDA_NAME: _ZN8WongWork15CCompoundAvatar17_ProcCompoundCoreEP5CUserRNS0_15tagCompoundInfoEP22tagCompoundAvatarTable
// DEMANGLED: WongWork::CCompoundAvatar::_ProcCompoundCore(CUser*, WongWork::CCompoundAvatar::tagCompoundInfo&, tagCompoundAvatarTable*)
int WongWork::CCompoundAvatar::_ProcCompoundCore(
    CUser* user,
    tagCompoundInfo& info,
    tagCompoundAvatarTable* table) {
    CInventory* const inventory = user->getCurCharacInvenW();
    const tagCompoundAvatar* const table_info = table->getCompoundAvatarInfo(
        info.compound_grade);
    const int equipment_type = GetItemType(info.first_avatar_prototype);
    int result_item_id = 0;

    if (!info.upper_compound || info.compound_kind == 4) {
        if (info.compound_kind == 4) {
            result_item_id = getItemIndex(
                AncientRareItems(table_info->ancient_rare_cube, equipment_type),
                static_cast<int>(get_rand_int(static_cast<int>(AncientRareCount(
                    table_info->ancient_rare_cube, equipment_type)))));
        } else {
            result_item_id = info.requested_result_item;
        }
    } else {
        const int rare_total =
            table_info->rare_avatar_total_rate_by_part[equipment_type];
        if (!rare_total || table_info->rare_avatar_items[equipment_type].empty()) {
            return 1;
        }

        int rare_rate = 0;
        if (info.first_avatar_prototype->get_grade() == 1 &&
            info.second_avatar_prototype->get_grade() == 1) {
            const unsigned long ten_thousand = 10000;
            if (random_->randInt(ten_thousand) <= 0x1387u) {
                const unsigned long total = static_cast<unsigned long>(rare_total);
                result_item_id = getItemIndex(
                    table_info->normal_avatar_items[equipment_type],
                    static_cast<int>(random_->randInt(total)));
            }
            if (result_item_id > 0) {
                CItem* const result_item = G_CDataManager()->find_item(
                    result_item_id);
                if (!result_item || !result_item->vtable_->is_avatar_item(
                                        result_item)) {
                    return 1;
                }
                const CEquipItem* const result_avatar =
                    static_cast<const CEquipItem*>(result_item);
                const int option_count = result_avatar->getAvatarOptionSize();
                const int option_key = option_count > 0
                    ? result_avatar->getAvatarOptionKeyByOrdinal(
                          get_rand_int(option_count))
                    : -1;
                info.requested_avatar_option =
                    option_key >= 0 && option_key <= 0xff
                        ? static_cast<uint16_t>(option_key)
                        : 0u;
            } else {
                rare_rate = table_info->rare_rate_by_part[equipment_type];
            }
        } else {
            rare_rate = table_info->upper_rare_rate_by_part[equipment_type];
        }

        if (result_item_id <= 0) {
            if (info.compound_kind == 1) {
                rare_rate += G_CDataManager()->
                    get_compound_avatar_upper_rate_bonus();
            }
            if (info.compound_kind == 2) {
                rare_rate += G_CDataManager()->
                    get_compound_avatar_master_rate_bonus();
            }
            if (info.compound_kind == 3) {
                rare_rate -= rare_rate * G_CDataManager()->
                    get_compound_avatar_lower_rate_reduction() / 10000;
            }
            const unsigned long ten_thousand = 10000;
            if (static_cast<int>(random_->randInt(ten_thousand)) < rare_rate) {
                result_item_id = getItemIndex(
                    table_info->rare_avatar_items[equipment_type],
                    static_cast<int>(get_rand_int(rare_total)));
            }
        }
    }

    if (!result_item_id) {
        result_item_id = getItemIndex(
            table_info->normal_avatar_items[equipment_type],
            static_cast<int>(get_rand_int(
                table_info->normal_avatar_total_rate_by_part[equipment_type])));
    }
    CItem* const result_item = result_item_id
        ? G_CDataManager()->find_item(result_item_id)
        : 0;
    if (!result_item || !result_item->vtable_->is_avatar_item(result_item)) {
        return 1;
    }

    // The 0828 request carries the explicit sparse ability key.  Validate it
    // against the final result prototype before deleting either source avatar
    // or any material.  Avatars without an ability table use the official
    // default key 0; 0xff is not a compound-avatar sentinel.
    const CEquipItem* const result_avatar =
        static_cast<const CEquipItem*>(result_item);
    const int result_option_count = result_avatar->getAvatarOptionSize();
    if ((result_option_count == 0 && info.requested_avatar_option != 0u) ||
        (result_option_count > 0 &&
         !result_avatar->IsAvatarOptionValid(
             static_cast<char>(info.requested_avatar_option)))) {
        return 17;
    }

    if (inventory->delete_item(static_cast<INVEN_TYPE>(2),
                               static_cast<int>(info.first_avatar_slot),
                               1,
                               static_cast<eItemDelReason>(3),
                               true) != 1 ||
        inventory->delete_item(static_cast<INVEN_TYPE>(2),
                               static_cast<int>(info.second_avatar_slot),
                               1,
                               static_cast<eItemDelReason>(3),
                               true) != 1) {
        return 22;
    }
    const int material_result = DeleteCompoundMaterials(inventory,
                                                        info,
                                                        *table_info);
    if (material_result != 0) {
        return material_result;
    }

    const int current_time = GlobalData::s_systemTime_.getCurSec();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        user->getCurCharacInvenR()->GetAvatarItemMgrR();
    const int first_key = info.first_avatar.get_add_info();
    const int second_key = info.second_avatar.get_add_info();
    const int first_remain = avatar_manager->GetRemainDate(first_key,
                                                            current_time);
    const int second_remain = avatar_manager->GetRemainDate(second_key,
                                                             current_time);

    WongWork::CAvatarItemMgr* const writable_avatar_manager =
        inventory->GetAvatarItemMgrW();
    writable_avatar_manager->UnRegistItem(first_key);
    inventory->SendAvatarEvent(54,
                               first_key,
                               1,
                               static_cast<int>(info.first_avatar_slot) +
                                   oldsrv::config::kAvatarInventorySlotBase,
                               0);
    info.avatar_mutations.push_back(1);
    info.avatar_mutations.push_back(info.first_avatar_slot);
    info.avatar_mutations.push_back(1);
    writable_avatar_manager->UnRegistItem(second_key);
    inventory->SendAvatarEvent(54,
                               second_key,
                               1,
                               static_cast<int>(info.second_avatar_slot) +
                                   oldsrv::config::kAvatarInventorySlotBase,
                               0);
    info.avatar_mutations.push_back(1);
    info.avatar_mutations.push_back(info.second_avatar_slot);
    info.avatar_mutations.push_back(1);

    char ipg_agency_no[0x18] = {};
    _generateIPGNo(user, current_time, ipg_agency_no);
    info.new_avatar_slot = static_cast<uint16_t>(inventory->AddAvatarItem(
        result_item_id,
        first_remain + second_remain,
        false,
        static_cast<char>(info.requested_avatar_option),
        static_cast<char>(-1),
        ipg_agency_no,
        static_cast<eAvatarItemAddReason>(6),
        true,
        0));
    if (info.new_avatar_slot == 0xffffu) {
        return 17;
    }

    DB_UpdateAvatarHistory::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        first_key,
        ipg_agency_no);
    DB_UpdateAvatarHistory::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        second_key,
        ipg_agency_no);
    return 0;
}

// IDA_EA: 0x83338C2
// IDA_NAME: _ZN8WongWork15CCompoundAvatar12ProcCompoundEP5CUserPK19MSG_COMPOUND_AVATAR
// DEMANGLED: WongWork::CCompoundAvatar::ProcCompound(CUser*, MSG_COMPOUND_AVATAR const*)
void WongWork::CCompoundAvatar::ProcCompound(
    CUser* user,
    const MSG_COMPOUND_AVATAR* request) {
    tagCompoundInfo info = _MakeCompoundInfo(user, request);
    int error = _CheckCondition(user, info);
    if (error != 0) {
        throw static_cast<ENUM_ERROR>(error);
    }
    error = _ProcCompoundCore(user, info, table_for_job(info.character_job));
    if (error != 0) {
        throw static_cast<ENUM_ERROR>(error);
    }
    _SendResult(user, info);
    _SendDisjointAvatarStatistic(info);
}

// IDA_EA: 0x8333A76
// IDA_NAME: _ZN8WongWork15CCompoundAvatar10_SendErrorEP5CUser10ENUM_ERROR
// DEMANGLED: WongWork::CCompoundAvatar::_SendError(CUser*, ENUM_ERROR)
int WongWork::CCompoundAvatar::_SendError(CUser* user, ENUM_ERROR error) {
    return user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_AVATAR,
                                    static_cast<unsigned char>(error));
}

// IDA_EA: 0x8333A9C
// IDA_NAME: _ZN8WongWork15CCompoundAvatar11_SendResultEP5CUserRKNS0_15tagCompoundInfoE
// DEMANGLED: WongWork::CCompoundAvatar::_SendResult(CUser*, WongWork::CCompoundAvatar::tagCompoundInfo const&)
int WongWork::CCompoundAvatar::_SendResult(CUser* user,
                                           const tagCompoundInfo& info) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_COMPOUND_AVATAR);
    response.put_byte(1);
    response.put_byte(static_cast<int>(info.avatar_mutations.size() / 3));
    for (std::vector<int>::const_iterator mutation =
             info.avatar_mutations.begin();
         mutation != info.avatar_mutations.end();) {
        response.put_byte(*mutation++);
        response.put_short(*mutation++);
        response.put_int(*mutation++);
    }

    const Inven_Item created = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(2), static_cast<int>(info.new_avatar_slot));
    response.put_short(info.new_avatar_slot);
    response.put_int(created.item_id_);
    const int current_time = GlobalData::s_systemTime_.getCurSec();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        user->getCurCharacInvenR()->GetAvatarItemMgrR();
    const int avatar_key = created.get_add_info();
    response.put_int(avatar_manager->GetRemainDate(avatar_key, current_time));
    response.put_short(created.avatar_select_);
    response.put_int(30);
    response.put_binary(
        reinterpret_cast<const char*>(
            avatar_manager->getJewelSocketData(avatar_key)),
        30);
    response.put_int(4);
    const stAvatarExpansionInfo_t* const expansion =
        avatar_manager->GetExpansionInfo(avatar_key);
    if (expansion) {
        response.put_binary(reinterpret_cast<const char*>(expansion), 4);
    } else {
        stAvatarExpansionInfo_t empty_expansion;
        empty_expansion.init();
        response.put_binary(reinterpret_cast<const char*>(&empty_expansion), 4);
    }
    response.finalize(true);
    user->Send(response);
    return 1;
}

// IDA_EA: 0x8333DF6
// IDA_NAME: _ZNK8WongWork15CCompoundAvatar14_generateIPGNoEPK5CUserlPc
// DEMANGLED: WongWork::CCompoundAvatar::_generateIPGNo(CUser const*, long, char*)const
char* WongWork::CCompoundAvatar::_generateIPGNo(const CUser* user,
                                                 long timer,
                                                 char* output) const {
    const time_t current_time = static_cast<time_t>(timer);
    tm local_time;
    localtime_r(&current_time, &local_time);
    std::snprintf(output,
                  0x14u,
                  "C%02d%02d%02d%02d%010d",
                  G_CEnvironment()->get_channel_no(),
                  (local_time.tm_year + 1900) % 2000,
                  local_time.tm_mon + 1,
                  local_time.tm_mday,
                  user->getCurCharacNo());
    return output;
}

// IDA_EA: 0x8333ECC
// IDA_NAME: _ZN8WongWork15CCompoundAvatar28_SendDisjointAvatarStatisticERNS0_15tagCompoundInfoE
// DEMANGLED: WongWork::CCompoundAvatar::_SendDisjointAvatarStatistic(WongWork::CCompoundAvatar::tagCompoundInfo&)
int WongWork::CCompoundAvatar::_SendDisjointAvatarStatistic(
    tagCompoundInfo& info) {
    Packet_Avater_Disjoint_Statistic packet;
    packet.action_0a = 2;
    packet.first_grade_bucket_0e = GradeBucket(info.first_avatar_prototype);
    packet.first_item_type_12 = GetItemType(info.first_avatar_prototype);
    packet.first_succeeded_16 = 0;
    packet.second_grade_bucket_1b = GradeBucket(info.second_avatar_prototype);
    // The original body reads the first prototype again for this second item
    // type field; retain the observed call sequence rather than normalising it.
    packet.second_item_type_1f = GetItemType(info.first_avatar_prototype);
    packet.second_succeeded_23 = 0;
    return GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x8333FDA
// IDA_NAME: _ZN8WongWork15CCompoundAvatar8ProcTestE17ENUM_CHARACTERJOB18ENUM_EQUIPMENTTYPEiiRSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: WongWork::CCompoundAvatar::ProcTest(ENUM_CHARACTERJOB, ENUM_EQUIPMENTTYPE, int, int, std::map<int,int>&)
bool WongWork::CCompoundAvatar::ProcTest(int character_job,
                                         int equipment_type,
                                         int first,
                                         int second,
                                         std::map<int, int>& result) {
    const tagCompoundAvatar* const table_info = table_for_job(character_job)
        ->getCompoundAvatarInfo(1);
    for (int count = 0; count <= 9999; ++count) {
        int result_item_id = 0;
        int rare_rate = table_info->rare_rate_by_part[equipment_type] +
            table_info->upper_rare_rate_by_part[equipment_type] * second;
        if (first == 1) {
            rare_rate += G_CDataManager()->get_compound_avatar_upper_rate_bonus();
        }
        const unsigned long ten_thousand = 10000;
        if (static_cast<int>(random_->randInt(ten_thousand)) < rare_rate) {
            result_item_id = getItemIndex(
                table_info->rare_avatar_items[equipment_type],
                static_cast<int>(get_rand_int(
                    table_info->rare_avatar_total_rate_by_part[equipment_type])));
        }
        if (!result_item_id) {
            result_item_id = getItemIndex(
                table_info->normal_avatar_items[equipment_type],
                static_cast<int>(get_rand_int(
                    table_info->normal_avatar_total_rate_by_part[equipment_type])));
        }
        ++result[result_item_id];
    }
    // The recovered body assigns false when its post-loop index reaches
    // 10000; preserve that return while still producing all 10000 samples.
    return false;
}
