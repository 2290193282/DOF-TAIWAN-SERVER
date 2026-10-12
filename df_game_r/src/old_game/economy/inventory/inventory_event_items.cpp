#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <cstring>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"
#include "common/manager/global_data.h"

namespace {

inline __attribute__((always_inline))
void SetItemId(Inven_Item& item, int item_id) {
    item.item_id_ = item_id;
}

inline __attribute__((always_inline))
int GetItemId(const Inven_Item& item) {
    return item.item_id_;
}

inline __attribute__((always_inline))
void SetItemAddInfo(Inven_Item& item, int add_info) {
    item.add_info_ = add_info;
}

inline __attribute__((always_inline))
int GetItemAddInfo(const Inven_Item& item) {
    return item.add_info_;
}

inline __attribute__((always_inline))
void PushInsertedItem(std::vector<int>& output,
                      int slot,
                      int item_id,
                      int add_info) {
    output.push_back(slot);
    output.push_back(item_id);
    output.push_back(add_info);
}

inline __attribute__((always_inline))
void PushDeletedItem(std::vector<int>& output,
                     int inventory_kind,
                     int slot,
                     int count_or_add_info) {
    output.push_back(inventory_kind);
    output.push_back(slot);
    output.push_back(count_or_add_info);
}

inline __attribute__((always_inline))
void SendEventItemMail(CUser* user,
                       const Inven_Item& item,
                       unsigned int money,
                       const char* success_message,
                       const char* failure_message) {
    char title[21];
    char text[256];
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, success_message, 0),
                 20u);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, failure_message, 0),
                 255u);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        money,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

inline __attribute__((always_inline))
CItemList* DataManagerItemList(CDataManager* data_manager) {
    return data_manager->get_item_list();
}

}  // namespace

// IDA_EA: 0x8505DB4
// IDA_NAME: _ZN10CInventory18delete_event_itemsERKSt6vectorISt4pairIiiESaIS2_EERS0_IiSaIiEERb15eMoneySubReason14eItemDelReasonb
// DEMANGLED: CInventory::delete_event_items(std::vector<std::pair<int,int> > const&,std::vector<int>&,bool&,eMoneySubReason,eItemDelReason,bool)
int CInventory::delete_event_items(
    const std::vector<std::pair<int, int> >& items,
    std::vector<int>& deleted_slots,
    bool& equipment_changed,
    eMoneySubReason money_reason,
    eItemDelReason item_reason,
    bool cube_failure) {
    CUser* const user = parent_;
    if (user && user->getGmQuestFlag()) {
        return 1;
    }

    int deleted_count = 0;
    if (items.empty()) {
        return deleted_count;
    }

    for (std::vector<std::pair<int, int> >::const_iterator requested =
             items.begin();
         requested != items.end();
         ++requested) {
        const int item_id = requested->first;
        int remaining = requested->second;

        if (item_id == 0) {
            if (use_money(remaining, money_reason, true)) {
                ++deleted_count;
                PushDeletedItem(deleted_slots, 0, 0, remaining);
            }
            if (static_cast<int>(money_reason) == 4 && user) {
                GetInstanceValueStatistic()->AddValueStatistic(
                    static_cast<VALUE_STATISTIC_FIELD>(17),
                    user,
                    static_cast<unsigned int>(remaining));
            }
            continue;
        }

        if (item_id == 2) {
            if (user && user->GetWinPoint() >= remaining) {
                user->UseWinPoint(
                    remaining, static_cast<eWPSubReason>(3));
                ++deleted_count;
                PushDeletedItem(deleted_slots, 0, 2, remaining);
            }
            continue;
        }

        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type),
                    start,
                    end);
        bool found = false;

        if (item_type == 1) {
            for (int slot = start; slot <= end && remaining > 0; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item && GetItemId(*item) == item_id) {
                    ++deleted_count;
                    PushDeletedItem(
                        deleted_slots, 0, slot, GetItemAddInfo(*item));
                    delete_item(static_cast<INVEN_TYPE>(1),
                                slot,
                                1,
                                item_reason,
                                true);
                    --remaining;
                }
            }
            for (int slot = 3; slot <= 8 && remaining > 0; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item && GetItemId(*item) == item_id) {
                    ++deleted_count;
                    PushDeletedItem(
                        deleted_slots, 0, slot, GetItemAddInfo(*item));
                    delete_item(static_cast<INVEN_TYPE>(1),
                                slot,
                                1,
                                item_reason,
                                true);
                    --remaining;
                }
            }
            for (int slot = 0;
                 slot < oldsrv::config::kEquipmentTypeCount &&
                     remaining > 0;
                 ++slot) {
                const Inven_Item* const item = GetInvenRef(0, slot);
                if (item && GetItemId(*item) == item_id) {
                    if (slot <= 11) {
                        equipment_changed = true;
                    }
                    ++deleted_count;
                    PushDeletedItem(
                        deleted_slots, 3, slot, GetItemAddInfo(*item));
                    delete_item(static_cast<INVEN_TYPE>(0),
                                slot,
                                1,
                                item_reason,
                                true);
                    --remaining;
                }
            }
            found = remaining == 0;
        } else {
            for (int pass = 0; pass != 2 && !found; ++pass) {
                const int first_slot = pass == 0 ? 3 : start;
                const int last_slot = pass == 0 ? 8 : end;
                for (int slot = first_slot; slot <= last_slot; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (!item ||
                        GetItemId(*item) != item_id ||
                        GetItemAddInfo(*item) < remaining) {
                        continue;
                    }

                    delete_item(static_cast<INVEN_TYPE>(1),
                                slot,
                                remaining,
                                item_reason,
                                true);
                    found = true;
                    ++deleted_count;
                    PushDeletedItem(deleted_slots, 0, slot, remaining);
                    if (user && static_cast<int>(item_reason) == 8) {
                        GetInstanceCubeStatistic()->collectCubeStatistics(
                            item_id,
                            remaining,
                            user,
                            static_cast<CUBE_STATISTIC_FIELD>(
                                cube_failure ? 101 : 100));
                    } else if (user &&
                               static_cast<int>(item_reason) == 4) {
                        GetInstanceCubeStatistic()->collectCubeStatistics(
                            item_id,
                            remaining,
                            user,
                            static_cast<CUBE_STATISTIC_FIELD>(102));
                    }
                    break;
                }
            }
        }

        if (!found) {
            cMyTrace trace(
                "int CInventory::delete_event_items(const std::vector<std::pair<int, int>, std::allocator<std::pair<int, int> > >&, std::vector<int, std::allocator<int> >&, bool&, eMoneySubReason, eItemDelReason, bool)",
                7482,
                5);
            trace("NOT CRITICAL BUT FOR OBSERVING, No Event Item %d FOUND!",
                  item_id);
            return 0;
        }
    }

    return deleted_count == static_cast<int>(items.size())
               ? deleted_count
               : 0;
}

// IDA_EA: 0x8507ACC
// IDA_NAME: _ZNK10CInventory17CheckNeedItemListERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CInventory::CheckNeedItemList(std::vector<std::pair<int,int> > const&)const
int CInventory::CheckNeedItemList(
    const std::vector<std::pair<int, int> >& items) const {
    CUser* const user = parent_;
    if (user && user->getGmQuestFlag()) {
        return 1;
    }
    if (items.empty()) {
        return 1;
    }

    for (std::vector<std::pair<int, int> >::const_iterator requested =
             items.begin();
         requested != items.end();
         ++requested) {
        const int item_id = requested->first;
        const int required = requested->second;
        if (item_id == 0) {
            if (get_money() < required) {
                return 0;
            }
            continue;
        }
        if (item_id == 2) {
            if (!user || user->GetWinPoint() < required) {
                return 0;
            }
            continue;
        }

        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }
        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type),
                    start,
                    end);

        if (item_type == 1) {
            int count = 0;
            for (int slot = start; slot <= end && count < required; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item && GetItemId(*item) == item_id) {
                    ++count;
                }
            }
            for (int slot = 3; slot <= 8 && count < required; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item && GetItemId(*item) == item_id) {
                    ++count;
                }
            }
            for (int slot = 0;
                 slot < oldsrv::config::kEquipmentTypeCount &&
                     count < required;
                 ++slot) {
                const Inven_Item* const item = GetInvenRef(0, slot);
                if (item && GetItemId(*item) == item_id) {
                    ++count;
                }
            }
            if (count < required) {
                return 0;
            }
            continue;
        }

        if (item_type == 5) {
            const Inven_Item* const equipped = GetInvenRef(
                0, oldsrv::config::kEquipmentTypeCreature);
            if (equipped && GetItemId(*equipped) == item_id) {
                continue;
            }
            bool found = false;
            for (int slot = start; slot <= end; ++slot) {
                const Inven_Item* const item = GetInvenRef(3, slot);
                if (item && GetItemId(*item) == item_id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return 0;
            }
            continue;
        }

        bool found = false;
        for (int slot = start; slot <= end; ++slot) {
            const Inven_Item* const item = GetInvenRef(1, slot);
            if (item && GetItemId(*item) == item_id) {
                if (GetItemAddInfo(*item) < required) {
                    return 0;
                }
                found = true;
                break;
            }
        }
        for (int slot = 3; slot <= 8 && !found; ++slot) {
            const Inven_Item* const item = GetInvenRef(1, slot);
            if (item && GetItemId(*item) == item_id) {
                if (GetItemAddInfo(*item) < required) {
                    return 0;
                }
                found = true;
            }
        }
        if (!found) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x850C8A8
// IDA_NAME: _ZNK10CInventory37GetExpertJobCompoundMaterialVariationE20ENUM_EXPERT_JOB_TYPEi
// DEMANGLED: CInventory::GetExpertJobCompoundMaterialVariation(ENUM_EXPERT_JOB_TYPE,int)const
float CInventory::GetExpertJobCompoundMaterialVariation(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level) const {
    float variation = 0.0f;
    if (static_cast<int>(expert_job_type) == 0) {
        return variation;
    }

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || GetItemId(*equipped) == 0) {
            continue;
        }
        CItem* const item =
            G_CDataManager()->find_item(GetItemId(*equipped));
        if (item->isExpertJobUsable(expert_job_type, expert_job_level)) {
            variation +=
                item->GetExpertJobCompoundMaterialVariation();
        }
    }
    return variation;
}

// IDA_EA: 0x850C95A
// IDA_NAME: _ZNK10CInventory33GetExpertJobCompoundRateVariationE20ENUM_EXPERT_JOB_TYPEi
// DEMANGLED: CInventory::GetExpertJobCompoundRateVariation(ENUM_EXPERT_JOB_TYPE,int)const
float CInventory::GetExpertJobCompoundRateVariation(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level) const {
    float variation = 0.0f;
    if (static_cast<int>(expert_job_type) == 0) {
        return variation;
    }

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || GetItemId(*equipped) == 0) {
            continue;
        }
        CItem* const item =
            G_CDataManager()->find_item(GetItemId(*equipped));
        if (item &&
            item->isExpertJobUsable(expert_job_type, expert_job_level)) {
            variation += item->GetExpertJobCompoundRateVariation();
        }
    }
    return variation;
}

// IDA_EA: 0x850CAE2
// IDA_NAME: _ZNK10CInventory34GetExpertJobSelfDisjointBigWinRateE20ENUM_EXPERT_JOB_TYPEi
float CInventory::GetExpertJobSelfDisjointBigWinRate(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level) const {
    float variation = 0.0f;
    if (static_cast<int>(expert_job_type) == 0) return variation;

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || GetItemId(*equipped) == 0) continue;
        CItem* const item = G_CDataManager()->find_item(GetItemId(*equipped));
        if (item && item->isExpertJobUsable(expert_job_type, expert_job_level))
            variation += item->GetExpertJobSelfDisjointBigWinRate();
    }
    return variation;
}

// IDA_EA: 0x850CB94
// IDA_NAME: _ZNK10CInventory39GetExpertJobSelfDisjointResultVariationE20ENUM_EXPERT_JOB_TYPEi
float CInventory::GetExpertJobSelfDisjointResultVariation(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level) const {
    float variation = 0.0f;
    if (static_cast<int>(expert_job_type) == 0) return variation;

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || GetItemId(*equipped) == 0) continue;
        CItem* const item = G_CDataManager()->find_item(GetItemId(*equipped));
        if (item && item->isExpertJobUsable(expert_job_type, expert_job_level))
            variation += item->GetExpertJobSelfDisjointResultVariation();
    }
    return variation;
}

// IDA_EA: 0x850CC46
// IDA_NAME: _ZNK10CInventory24GetExpertJobAddtionalExpE20ENUM_EXPERT_JOB_TYPEiRN12STItemScript10SEXPERTJOB24stExpertJobAdditionalExpE
// DEMANGLED: CInventory::GetExpertJobAddtionalExp(ENUM_EXPERT_JOB_TYPE,int,STItemScript::SEXPERTJOB::stExpertJobAdditionalExp &)const
void CInventory::GetExpertJobAddtionalExp(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level,
    STItemScript::SEXPERTJOB::stExpertJobAdditionalExp& value) const {
    if (static_cast<int>(expert_job_type) == 0) {
        return;
    }

    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || GetItemId(*equipped) == 0) {
            continue;
        }
        CItem* const item =
            G_CDataManager()->find_item(GetItemId(*equipped));
        if (!item->isExpertJobUsable(
                expert_job_type, expert_job_level)) {
            continue;
        }
        STItemScript::SEXPERTJOB::stExpertJobAdditionalExp
            item_value;
        item->GetExpertJobAdditionalExp(item_value);
        value.value_00 += item_value.value_00;
        value.value_04 += item_value.value_04;
    }
}

// IDA_EA: 0x8506BB2
// IDA_NAME: _ZN10CInventory18insert_event_itemsERKSt6vectorISt4pairIiiESaIS2_EERS0_IiSaIiEE15eMoneyAddReason14eItemAddReasonbPKcSD_
// DEMANGLED: CInventory::insert_event_items(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector&<int,std::allocator<int>>,eMoneyAddReason,eItemAddReason,bool,char const*,char const*)
int CInventory::insert_event_items(
    const std::vector<std::pair<int, int> >& items,
    std::vector<int>& inserted_slots,
    eMoneyAddReason money_reason,
    eItemAddReason item_reason,
    bool notify,
    const char* success_message,
    const char* failure_message) {
    int inserted_count = 0;
    if (items.empty()) {
        return inserted_count;
    }

    CUser* const user = parent_;
    Inven_Item item;
    int accumulated_value = 0;

    for (std::vector<std::pair<int, int> >::const_iterator event_item =
             items.begin();
         event_item != items.end();
         ++event_item) {
        item.reset();
        const int requested_item_id = event_item->first;
        const int requested_count = event_item->second;

        if (requested_item_id == 0) {
            if (requested_count > 499999 && user) {
                cMyTrace trace(
                    "int CInventory::insert_event_items(const std::vector<std::pair<int, int>, std::allocator<std::pair<int, int> > >&, std::vector<int, std::allocator<int> >&, eMoneyAddReason, eItemAddReason, bool, const char*, const char*)",
                    7623,
                    4);
                trace("Quest reward money(%d) mid(%s) characno(%d) reason(%d)",
                      requested_count,
                      NumberToString(
                          static_cast<unsigned int>(user->get_acc_id()), 0),
                      user->get_charac_no(-1),
                      static_cast<int>(money_reason));
            }

            int gained_money = requested_count;
            if (user && user->getCurCharacR()) {
                gained_money = user->getCurCharacInvenW()->gain_money(
                    requested_count, money_reason, true, 0);
            }
            PushInsertedItem(inserted_slots, 0, 0, gained_money);
            ++inserted_count;
            if (money_reason == static_cast<eMoneyAddReason>(3)) {
                accumulated_value += gained_money;
            }
            if (requested_count > gained_money && notify) {
                SendEventItemMail(user,
                                  item,
                                  static_cast<unsigned int>(
                                      requested_count - gained_money),
                                  success_message,
                                  failure_message);
            }
            continue;
        }

        CItem* const prototype =
            G_CDataManager()->find_item(requested_item_id);
        if (!prototype) {
            cMyTrace trace(
                "int CInventory::insert_event_items(const std::vector<std::pair<int, int>, std::allocator<std::pair<int, int> > >&, std::vector<int, std::allocator<int> >&, eMoneyAddReason, eItemAddReason, bool, const char*, const char*)",
                7700,
                5);
            trace("CInventory::insert_event_items, G_CDataManager()->find_item(%d)",
                  requested_item_id);
            continue;
        }

        SetItemId(item, requested_item_id);
        const CItemVTable* const vtable = prototype->vtable_;
        if (prototype->is_stackable()) {
            SetItemAddInfo(item, requested_count);
            vtable->make_item(prototype, item);
            const int slot = insertItemIntoInventory(
                item, item_reason, true, true);
            if (slot == -1) {
                if (!notify) {
                    break;
                }
                SendEventItemMail(user,
                                  item,
                                  0,
                                  success_message,
                                  failure_message);
                continue;
            }

            ++inserted_count;
            PushInsertedItem(inserted_slots,
                             slot,
                             GetItemId(item),
                             GetItemAddInfo(item));
            if (item_reason == static_cast<eItemAddReason>(8)) {
                if (user) {
                    GetInstanceCubeStatistic()->collectCubeStatistics(
                        prototype,
                        GetItemAddInfo(item),
                        user,
                        static_cast<CUBE_STATISTIC_FIELD>(4));
                }
                int sell_price = 0;
                G_Store()->GetSellItemPrice(
                    item,
                    prototype,
                    static_cast<short>(GetItemAddInfo(item)),
                    false,
                    sell_price);
                accumulated_value += sell_price;
            }
            continue;
        }

        if (vtable->is_avatar_item(prototype)) {
            if (item_reason != static_cast<eItemAddReason>(8)) {
                continue;
            }
            char ipg_no[24];
            WongWork::CGenUniqueNo::genIPGNo(
                static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(2),
                static_cast<unsigned int>(user->getCurCharacNo()),
                ipg_no);
            const int slot = AddAvatarItem(
                requested_item_id,
                prototype->getUsablePeriod(),
                false,
                0,
                -1,
                ipg_no,
                static_cast<eAvatarItemAddReason>(8),
                false,
                0);
            if (slot == -1) {
                if (!notify) {
                    break;
                }
                WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(
                    user,
                    user->getCurCharacNo(),
                    requested_item_id,
                    0,
                    0,
                    -1,
                    ipg_no,
                    0,
                    0);
                continue;
            }

            ++inserted_count;
            const Inven_Item avatar = GetInvenSlot(2, slot);
            const int key = GetItemAddInfo(avatar);
            PushInsertedItem(
                inserted_slots,
                slot,
                requested_item_id,
                avatar_item_manager_.GetRemainDate(
                    key, GlobalData::s_systemTime_.getCurSec()));
            continue;
        }

        if (vtable->is_creature_item(prototype) &&
            vtable->get_item_type(prototype) ==
                oldsrv::config::kEquipmentTypeCreature) {
            if (item_reason != static_cast<eItemAddReason>(8)) {
                continue;
            }
            SetItemId(item, requested_item_id);
            DataManagerItemList(G_CDataManager())->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, item, requested_count);
            if (item.item_category_ != 5u ||
                !user_creature::CCreatureItemConverter::SetEggItem(&item)) {
                continue;
            }
            const int slot = insertItemIntoInventory(
                item, static_cast<eItemAddReason>(8), true, false);
            if (slot >= 0) {
                GetCreatureMgrW()->InsertCreatureItem(
                    &item,
                    slot,
                    8,
                    prototype->getUsablePeriod(),
                    prototype->getExpirationDate());
                ++inserted_count;
                PushInsertedItem(inserted_slots,
                                 slot,
                                 GetItemId(item),
                                 requested_count);
            }
            continue;
        }

        for (int index = 0; index < requested_count; ++index) {
            vtable->make_item(prototype, item);
            const int slot = insertItemIntoInventory(
                item, item_reason, true, true);
            if (slot == -1) {
                if (!notify) {
                    break;
                }
                SendEventItemMail(user,
                                  item,
                                  0,
                                  success_message,
                                  failure_message);
                continue;
            }

            ++inserted_count;
            PushInsertedItem(inserted_slots,
                             slot,
                             GetItemId(item),
                             GetItemAddInfo(item));
            if (item_reason == static_cast<eItemAddReason>(8)) {
                int sell_price = 0;
                G_Store()->GetSellItemPrice(
                    item,
                    prototype,
                    static_cast<short>(GetItemAddInfo(item)),
                    false,
                    sell_price);
                accumulated_value += sell_price;
            }
        }
    }

    if ((item_reason == static_cast<eItemAddReason>(8) ||
         money_reason == static_cast<eMoneyAddReason>(3)) &&
        user) {
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(10),
            user,
            static_cast<unsigned int>(accumulated_value));
    }
    return inserted_count;
}
#include "common/config.h"
