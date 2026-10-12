#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include <cstdint>
#include <cstring>
#include <vector>

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/packet_guard.h"

namespace {

float GetPartyQuestDropScale(const CDataManager* data_manager,
                             int member_count) {
    if (member_count <= 0) {
        return 1.0f;
    }

    return data_manager->get_server_parameter_script()
        ->get_quest_item_drop_bonus_starter_server(member_count);
}

void PutAvatarQuestItemMetadata(PacketGuard& packet,
                                const CUser& user,
                                const Inven_Item& item) {
    const WongWork::CAvatarItemMgr* const avatar_manager =
        user.getCurCharacInvenR()->GetAvatarItemMgrR();
    const int avatar_key = item.get_add_info();

    packet.put_int(30);
    packet.put_binary(
        reinterpret_cast<const char*>(
            avatar_manager->getJewelSocketData(avatar_key)),
        30);

    packet.put_int(4);
    const stAvatarExpansionInfo_t* const expansion =
        avatar_manager->GetExpansionInfo(avatar_key);
    if (expansion) {
        packet.put_binary(
            reinterpret_cast<const char*>(expansion), 4);
    } else {
        stAvatarExpansionInfo_t empty_expansion;
        empty_expansion.init();
        packet.put_binary(
            reinterpret_cast<const char*>(&empty_expansion), 4);
    }
}

void PutExistingQuestItemRecord(PacketGuard& packet,
                                const CUser& user,
                                int slot,
                                int item_id,
                                const Inven_Item& item,
                                bool avatar_item) {
    packet.put_short(slot);
    packet.put_int(item_id);
    packet.put_int(item.get_add_info());
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(0);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    if (avatar_item) {
        PutAvatarQuestItemMetadata(packet, user, item);
    }
    packet.put_byte(0);
    packet.put_packet(item);
}

void PutInsertedQuestItemRecord(PacketGuard& packet,
                                const CUser& user,
                                int slot,
                                int item_id,
                                const Inven_Item& item) {
    packet.put_short(slot);
    packet.put_int(item_id);
    packet.put_int(item.get_add_info());
    packet.put_byte(item.GetItemAttr());
    packet.put_short(item.avatar_select_);
    packet.put_byte(0);
    packet.put_int(item.extend_info_);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    if (item.isAvatarItemType()) {
        PutAvatarQuestItemMetadata(packet, user, item);
    }
    packet.put_byte(0);
    packet.put_packet(g_emptySlot);
}

void EraseSelectedMonsterKillItem(
    std::vector<MonsterKillItem>& matched_items,
    const MonsterKillItem& selected) {
    for (std::vector<MonsterKillItem>::iterator item =
             matched_items.begin();
         item != matched_items.end();
         ++item) {
        if (std::memcmp(&selected, &*item, sizeof(selected)) == 0) {
            matched_items.erase(item);
            return;
        }
    }
}

}  // namespace

// IDA_EA: 0x866CB04
// IDA_NAME: _ZN5CUser17CheckQuestMonsterEii21ENUM_QUEST_ENEMY_TYPE
// DEMANGLED: CUser::CheckQuestMonster(int,int,ENUM_QUEST_ENEMY_TYPE)
bool CUser::CheckQuestMonster(int dungeon_index,
                              int monster_index,
                              ENUM_QUEST_ENEMY_TYPE enemy_type) {
    UserQuest* const quest_state = getCurCharacQuestRefR();
    for (int quest_slot = 0; quest_slot <= 19; ++quest_slot) {
        const int quest_index =
            static_cast<int>(quest_state->play_quest[quest_slot]);
        if (quest_index <= 0) {
            continue;
        }

        Quest* const quest = G_CDataManager()->find_quest(quest_index);
        if (!quest) {
            LogManager::logFormat(
                1,
                "user.cpp",
                "void CUser::CheckQuestMonster(int, int, "
                "ENUM_QUEST_ENEMY_TYPE)",
                0x54F7,
                "G_CDataManager()->find_quest(%d) fail",
                quest_index);
            continue;
        }

        if (quest->quest_grade != 4 &&
            quest->quest_grade != 3 &&
            !quest_state->play_quest_trigger[quest_slot]) {
            continue;
        }

        std::vector<MonsterKillItem> matched_items;
        if (quest->CheckKillMonster(
                dungeon_index,
                static_cast<int>(quest_state->authen_data.clear_count),
                monster_index,
                enemy_type,
                matched_items)) {
            bool retry_with_another_item = false;
            do {
                retry_with_another_item = false;
                if (matched_items.empty()) {
                    break;
                }

                MonsterKillItem selected;
                const int random_index = static_cast<int>(
                    get_rand_int(
                        static_cast<int>(matched_items.size())));
                if (random_index < 0 ||
                    random_index >=
                        static_cast<int>(matched_items.size())) {
                    break;
                }
                selected = matched_items[random_index];

                const int item_id = selected.map_index;
                const int max_add_info = selected.value_1c;
                const int base_drop_rate = selected.value_18;
                int equipment_bonus = 0;
                for (int equipment_slot = 0;
                     equipment_slot < oldsrv::config::kEquipmentTypeCount;
                     ++equipment_slot) {
                    const Inven_Item* const equipped =
                        getCurCharacInvenR()->GetInvenRef(
                            0, equipment_slot);
                    if (!equipped || !equipped->item_id_) {
                        continue;
                    }
                    const unsigned long equipped_item_id =
                        static_cast<unsigned long>(
                            equipped->item_id_);
                    equipment_bonus +=
                        G_CDataManager()
                            ->get_item_list()
                            ->GetQuestItemDropBouns(equipped_item_id) *
                        base_drop_rate / 100;
                }

                int effective_drop_rate =
                    expert_job::CAlchemist::GetQuestItemDropRate(
                        this, base_drop_rate) +
                    equipment_bonus;
                if (G_CEnvironment()->get_server_type() == 2) {
                    CParty* const party = GetParty();
                    if (party) {
                        effective_drop_rate = static_cast<int>(
                            static_cast<long double>(
                                effective_drop_rate) *
                            GetPartyQuestDropScale(
                                G_CDataManager(),
                                party->get_member_count()));
                    }
                }
                if (isAffectedPremium(
                        static_cast<ENUM_PREMIUM_TYPE>(79))) {
                    effective_drop_rate +=
                        getAddQuestItemDropRateFromPremium(
                            static_cast<ENUM_PREMIUM_TYPE>(79)) /
                        100 * base_drop_rate;
                }

                int generated_count = 0;
                for (int roll = 0;
                     roll < selected.difficulty;
                     ++roll) {
                    if (static_cast<int>(get_rand_int(100)) <
                        effective_drop_rate) {
                        ++generated_count;
                    }
                }
                if (!generated_count) {
                    break;
                }

                Inven_Item generated_item;
                Inven_Item existing_item;
                generated_item.item_id_ =
                    static_cast<uint32_t>(item_id);
                CItem* const item_data =
                    G_CDataManager()->find_item(item_id);
                item_data->vtable_->make_item(
                    item_data, generated_item);
                generated_item.set_add_info(generated_count);
                if (generated_count > 999) {
                    cMyTrace trace(
                        "void CUser::CheckQuestMonster(int, int, "
                        "ENUM_QUEST_ENEMY_TYPE)",
                        21881,
                        5);
                    trace(
                        "CUser::CheckQuestMonster, "
                        "mon_item.item.set_add_info , ITEM #%d, %d , "
                        "QUEST#%d",
                        item_id,
                        generated_count,
                        quest_index);
                    generated_count = 1000;
                }

                PacketGuard packet;
                packet.put_header(0, 14);
                packet.put_byte(0);
                packet.put_short(1);

                const int existing_slot =
                    getCurCharacInvenR()->GetInvenData(
                        item_id, existing_item);
                if (existing_slot >= 0) {
                    if (max_add_info == -1) {
                        existing_item.set_add_info(
                            existing_item.get_add_info() +
                            generated_item.get_add_info());
                    } else if (
                        existing_item.get_add_info() >= max_add_info) {
                        EraseSelectedMonsterKillItem(
                            matched_items, selected);
                        retry_with_another_item = true;
                        continue;
                    } else if (
                        existing_item.get_add_info() +
                                generated_item.get_add_info() <
                            max_add_info) {
                        existing_item.set_add_info(
                            existing_item.get_add_info() +
                            generated_item.get_add_info());
                    } else {
                        existing_item.set_add_info(max_add_info);
                    }

                    getCurCharacInvenW()->update_item(
                        static_cast<INVEN_TYPE>(1),
                        existing_slot,
                        existing_item);
                    PutExistingQuestItemRecord(
                        packet,
                        *this,
                        existing_slot,
                        item_id,
                        existing_item,
                        generated_item.isAvatarItemType());
                    packet.finalize(true);
                    Send(packet);
                } else {
                    const int inserted_slot =
                        getCurCharacInvenW()->insertItemIntoInventory(
                            generated_item,
                            static_cast<eItemAddReason>(8),
                            true,
                            true);
                    if (inserted_slot > 0) {
                        PutInsertedQuestItemRecord(
                            packet,
                            *this,
                            inserted_slot,
                            item_id,
                            generated_item);
                        packet.finalize(true);
                        Send(packet);
                    }
                }
            } while (retry_with_another_item);
        }
    }
    return false;
}
