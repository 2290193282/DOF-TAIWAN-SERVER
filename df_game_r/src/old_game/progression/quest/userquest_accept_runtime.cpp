#include "old_game/progression/quest/quest_state_runtime.h"

#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/interface_packet_buf.h"

#include <cstring>
#include <utility>
#include <vector>

// IDA_EA: 0x86ABE31
// IDA_NAME: _Z20checkAcceptableQuestPK5QuestRKN8WongWork11CQuestClearERK18stSelectQuestParam
// DEMANGLED: checkAcceptableQuest(Quest const*,WongWork::CQuestClear const&,stSelectQuestParam const&)
int checkAcceptableQuest(const Quest* quest,
                         const WongWork::CQuestClear& cleared_quests,
                         const stSelectQuestParam& parameter) {
    if (quest->quest_index == 1016) {
        return 1;
    }
    if (quest->exposeQuest() != 1) {
        return 0;
    }
    if (!quest->check_possible(parameter)) {
        return 0;
    }

    QuestList* const quest_list =
        G_CDataManager()->get_arad_jumping_quest_list();
    if (!quest->isRepeatableQuest() &&
        !quest_list->check_clear(quest->quest_index, cleared_quests)) {
        return 0;
    }
    if (quest_list->check_ahead_quest(quest->quest_index, cleared_quests) !=
        1) {
        return 0;
    }
    return quest_list->check_anti_quest(quest->quest_index, cleared_quests) ==
           1;
}

// IDA_EA: 0x86ABF38
// IDA_NAME: _ZN9UserQuest12accept_questEiPcRi
// DEMANGLED: UserQuest::accept_quest(int,char*,int&)
int UserQuest::accept_quest(int quest_index,
                            char* packet_storage,
                            int& accepted_slot) {
    if (!parent) {
        LogManager::logFormat(1,
                              "user_quest.cpp",
                              "int UserQuest::accept_quest(int, char*, int&)",
                              0x1E8,
                              "user is null");
        return 1;
    }

    if (!parent->getCurCharacR()) {
        cMyTrace trace("int UserQuest::accept_quest(int, char*, int&)",
                       495,
                       5);
        trace("UserQuest::accept_quest user->m_selected is null");
        return 1;
    }

    Quest* const quest = G_CDataManager()->find_quest(quest_index);
    if (!quest) {
        LogManager::logFormat(1,
                              "user_quest.cpp",
                              "int UserQuest::accept_quest(int, char*, int&)",
                              0x1F8,
                              "G_CDataManager()->find_quest(%d)",
                              quest_index);
        return 1;
    }

    for (int slot = 0; slot <= 19; ++slot) {
        if (play_quest[slot] == static_cast<uint32_t>(quest_index)) {
            return 18;
        }
    }

    if (parent->IsGameMasterMode() != 1 && !quest->isRepeatableQuest() &&
        clear_quest.isClearedQuest(static_cast<unsigned int>(quest_index)) !=
            0) {
        return 18;
    }

    if (parent->IsGameMasterMode() != 1) {
        const stSelectQuestParam parameter(parent);
        const UserQuest* const current_quest = parent->getCurCharacQuestR();
        if (!checkAcceptableQuest(quest, current_quest->clear_quest,
                                  parameter)) {
            parent->getHackAnalyzer()->addServerHackCnt(
                parent,
                static_cast<WongWork::ENUM_HACKTYPE>(0x191u),
                1u,
                0u,
                0u);
            return 1;
        }
    }

    std::vector<std::pair<int, int> > requested_items =
        quest->depend_give_items;
    std::vector<std::pair<int, int> > remaining_items;
    const int repeated_item_result =
        parent->getCurCharacInvenW()->erase_repeated_item(requested_items,
                                                          remaining_items);
    if (repeated_item_result > 0) {
        int needed_slots[11];
        std::memset(needed_slots, 0, sizeof(needed_slots));

        CInventory* const inventory = parent->getCurCharacInvenR();
        inventory->CalNeedSlot(needed_slots, remaining_items);
        int available_quick_slots = inventory->QuickEmptyCount();
        for (int item_type = 1; item_type <= 10; ++item_type) {
            const int available_slots = inventory->InvenEmptycount(
                static_cast<Inven_Item::ITEM_TYPE>(item_type));
            if (needed_slots[item_type] > available_slots) {
                available_quick_slots += available_slots - needed_slots[item_type];
                if (available_quick_slots < 0) {
                    return 17;
                }
            }
        }
    }

    accepted_slot = quest->get_init_trigger();
    int quest_slot = 0;
    for (;; ++quest_slot) {
        if (quest_slot > 19) {
            return 4;
        }
        if (!play_quest[quest_slot]) {
            break;
        }
    }

    clear_quest.resetClearedQuests(static_cast<unsigned int>(quest_index));
    play_quest[quest_slot] = static_cast<uint32_t>(quest_index);
    play_quest_trigger[quest_slot] = static_cast<uint32_t>(accepted_slot);

    InterfacePacketBuf* const packet =
        reinterpret_cast<InterfacePacketBuf*>(packet_storage);
    packet->put_byte(1);
    packet->put_short(static_cast<short>(quest_index));
    packet->put_int(accepted_slot);

    std::vector<int> inserted_slots;
    const int inserted = parent->getCurCharacInvenW()->insert_event_items(
        remaining_items,
        inserted_slots,
        static_cast<eMoneyAddReason>(3),
        static_cast<eItemAddReason>(8),
        false,
        "",
        "");
    packet->put_byte(inserted);
    for (std::vector<int>::const_iterator it = inserted_slots.begin();
         it != inserted_slots.end();) {
        packet->put_short(*it++);
        packet->put_int(*it++);
        packet->put_int(*it++);
    }
    return 0;
}
