#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/db/event/db_update_happy_bean_info.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "common/manager/system_time.h"

#include <stdint.h>

// IDA_EA: 0x8664412
// IDA_NAME: _ZN5CUser13_onQuestClearEii
// DEMANGLED: CUser::_onQuestClear(int,int)
Stream* CUser::_onQuestClear(int quest_index, int /*trigger_value*/) {
    EventClassify::CEventScriptMng* const event_script_manager =
        G_CDataManager()->get_event_script_mng();
    if (static_cast<unsigned char>(
            event_script_manager->is_eventing(0x193)) != 0u) {
        G_CDataManager()->get_event_script_mng()->process_add_quest_count_db(
            this,
            quest_index);
    }

    if (quest_index == 2439 &&
        GlobalData::s_systemTime_.getCurSec() <= 1202936400) {
        DB_InsertArchieveEventLog::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            static_cast<unsigned int>(getCurCharacNo()),
            static_cast<SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE>(50));
    }

    if (quest_index == 2415 &&
        GlobalData::s_systemTime_.getCurSec() <= 1200517200) {
        DB_UpdateHappyBeanInfo::makeRequest(
            static_cast<unsigned int>(GetUID()),
            static_cast<unsigned int>(get_acc_id()),
            static_cast<unsigned int>(getCurCharacNo()),
            getCurCharacName());
    }

    if (quest_index != 2416) {
        return 0;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(19);
    if (static_cast<unsigned char>(repeat_event->IsEventing(0)) == 0u) {
        return 0;
    }

    return DB_Insert0712WinterEventCoupon::makeRequest(
        static_cast<unsigned int>(get_acc_id()),
        static_cast<unsigned int>(getCurCharacNo()),
        1u);
}

// IDA_EA: 0x8664DCE
// IDA_NAME: _ZN5CUser20complete_reward_listERSt6vectorISt4pairIiiESaIS2_EES5_
// DEMANGLED: CUser::complete_reward_list(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
bool CUser::complete_reward_list(
    std::vector<std::pair<int, int> >& reward_items,
    std::vector<std::pair<int, int> >& completed_items) {
    for (std::vector<std::pair<int, int> >::iterator reward =
             reward_items.begin();
         reward != reward_items.end();
         ++reward) {
        for (std::vector<std::pair<int, int> >::const_iterator completed =
                 completed_items.begin();
             completed != completed_items.end();
             ++completed) {
            if (reward->first == completed->first) {
                reward->second -= completed->second;
            }
        }
    }
    // The decompiled loop returns its terminating condition, which is false.
    return false;
}

// IDA_EA: 0x86650B2
// IDA_NAME: _ZN5CUser16add_matched_listERSt6vectorISt4pairIiiESaIS2_EEii
// DEMANGLED: CUser::add_matched_list(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int,int)
int CUser::add_matched_list(std::vector<std::pair<int, int> >& matched_items,
                            int item_id,
                            int item_count) {
    for (std::vector<std::pair<int, int> >::iterator item =
             matched_items.begin();
         item != matched_items.end();
         ++item) {
        if (item->first == item_id) {
            item->second += item_count;
            return static_cast<int>(reinterpret_cast<uintptr_t>(&*item));
        }
    }

    matched_items.push_back(std::make_pair(item_id, item_count));
    return static_cast<int>(
        reinterpret_cast<uintptr_t>(&matched_items.back()));
}

// IDA_EA: 0x866514A
// IDA_NAME: _ZN5CUser20count_specific_itemsERSt6vectorIiSaIiEERS0_ISt4pairIiiESaIS5_EE
// DEMANGLED: CUser::count_specific_items(std::vector<int,std::allocator<int>> &,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
bool CUser::count_specific_items(
    std::vector<int>& item_ids,
    std::vector<std::pair<int, int> >& item_counts) {
    CInventory* const inventory = getCurCharacInvenR();
    CCargo* const cargo = getCurCharacCargoR();
    Inven_Item item;

    for (std::vector<int>::const_iterator item_id = item_ids.begin();
         item_id != item_ids.end();
         ++item_id) {
        int count = 0;
        if (inventory->GetInvenData(*item_id, item) != -1) {
            count += item.add_info_;
        }
        if (cargo->GetCargoData(*item_id, item) != -1) {
            count += item.add_info_;
        }
        if (count > 0) {
            item_counts.push_back(std::make_pair(*item_id, count));
        }
    }
    // The raw loop returns its terminating condition, which is false.
    return false;
}

// IDA_EA: 0x8665274
// IDA_NAME: _ZN5CUser17apply_elixir_itemER15_Additioal_infoRSt4pairIiiE
// DEMANGLED: CUser::apply_elixir_item(_Additioal_info &,std::pair<int,int> &)
char CUser::apply_elixir_item(_Additioal_info& additional_info,
                              std::pair<int, int>& item) {
    const uint16_t item_count = static_cast<uint16_t>(item.second);
    switch (item.first) {
    case 0x40f:
        additional_info.physical_attack += 50 * item_count;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x410:
        additional_info.magical_attack += 50 * item_count;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x411:
        additional_info.physical_defense += 50 * item_count;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x412:
        additional_info.magical_defense += 50 * item_count;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x413:
        additional_info.max_hp += 250 * item.second;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x414:
        additional_info.max_mp += 250 * item.second;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x415:
        additional_info.move_speed += 10 * item.second;
        return static_cast<char>(reinterpret_cast<uintptr_t>(&additional_info));
    case 0x416:
        for (int index = 0; index <= 3; ++index) {
            additional_info.element_resist[index] += 10 * item_count;
        }
        return 0;
    default:
        return static_cast<char>(item.first - 1039);
    }
}
