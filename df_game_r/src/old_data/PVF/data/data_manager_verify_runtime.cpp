#include "old_data/PVF/data/data_manager.h"

#include "common/config.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/progression/quest/quest_training_lifecycle_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/sync_script.h"
#include "old_data/PVF/n_quest/training_quest_script.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"

#include <algorithm>
#include <climits>
#include <set>
#include <vector>

namespace {

const char kVerifyPrettyFunction[] = "bool CDataManager::Verify() const";
const char kReselectDailyTrainingQuestPrettyFunction[] =
    "bool CDataManager::reselectDailyTrainingQuest()";

bool MissingReferencedItem(int item_index) {
    return item_index != 1 && !G_CDataManager()->find_item(item_index);
}

}  // namespace

// IDA_EA: 0x8362B44
// IDA_NAME: _ZNK12CDataManager13VerifyDungeonERK8CDungeon
// DEMANGLED: CDataManager::VerifyDungeon(CDungeon const&)const
bool CDataManager::VerifyDungeon(const CDungeon& dungeon) const {
    for (std::map<int, int>::const_iterator it =
             dungeon.clear_item_rates_.begin();
         it != dungeon.clear_item_rates_.end(); ++it) {
        // Old code deliberately ignores this lookup result: it only retains
        // the lookup-side diagnostics/state access before maze validation.
        (void)find_item(it->second);
    }
    (void)dungeon.VerifyMaze();
    return true;
}

// IDA_EA: 0x8360D94
// IDA_NAME: _ZNK12CDataManager6VerifyEv
// DEMANGLED: CDataManager::Verify(void)const
bool CDataManager::Verify() const {
    // The initial 1..197 loop in the old body has no observable operation.
    // Its following reward pass compares exactly 200 adjacent table entries.
    for (int grade = 0; grade <= 199; ++grade) {
        const uint32_t reward = mob_rewards_[grade];
        if (mob_rewards_[grade + 1] <= reward) {
            cMyTrace trace(kVerifyPrettyFunction, 10019, 9, true, true);
            trace("Warning: Unusual mob reward(%d) on mob grade(%d)",
                  reward,
                  grade);
        }
    }

    bool skill_verification_failed = false;
    for (int job_type = 0;
         job_type < oldsrv::config::kCharacterJobCount;
         ++job_type) {
        const __gnu_cxx::hash_map<int, CSkill*>* const skills =
            skill_list_->getSkillList(job_type);
        for (__gnu_cxx::hash_map<int, CSkill*>::const_iterator it =
                 skills->begin();
             it != skills->end(); ++it) {
            CSkill* const skill = it->second;
            if (!skill->level_interval_) {
                cMyTrace trace(kVerifyPrettyFunction, 10030, 9, true, true);
                trace("Skill [required level range] tag value error "
                      "(Job: %d)(Skill: %d)",
                      job_type,
                      skill->get_index());
                skill_verification_failed = true;
            }
        }
    }
    if (skill_verification_failed) {
        cMyTrace trace(kVerifyPrettyFunction, 10037, 9, true, true);
        trace("Verifying Skills fail");
        return false;
    }

    // This is intentionally an assignment, rather than an accumulated
    // result.  The final hash-map element alone controls this old failure
    // gate, matching the 0x8361089 store in the server binary.
    bool map_verification_failed = false;
    for (CMapList::MapHash::const_iterator it = map_list_->maps_.begin();
         it != map_list_->maps_.end(); ++it) {
        map_verification_failed = !VerifyMap(*it->second, -1);
    }
    if (map_verification_failed) {
        return false;
    }

    for (CDungeonList::const_iterator it = dungeon_index_list_->begin();
         it != dungeon_index_list_->end(); ++it) {
        (void)VerifyDungeon(*it->second);
    }

    bool item_verification_failed = false;
    const CItemIndexList& items_by_index = item_list_->items_by_index_;
    for (CItemIndexList::const_iterator item_it = items_by_index.begin();
         item_it != items_by_index.end(); ++item_it) {
        CItem* const item = item_it->second;
        if (!item->is_stackable()) {
            continue;
        }

        CStackableItem* const stackable_item =
            static_cast<CStackableItem*>(item);
        if (item->vtable_->get_item_type(item) == 15 ||
            item->vtable_->get_item_type(item) == 16) {
            const stStackableBooster_t& booster =
                stackable_item->booster_data();
            for (std::vector<stStackableBoosterElement_t>::const_iterator
                     element_it = booster.elements.begin();
                 element_it != booster.elements.end(); ++element_it) {
                for (std::vector<stStackableBoosterItemInfo_t>::const_iterator
                         booster_item_it = element_it->items.begin();
                     booster_item_it != element_it->items.end();
                     ++booster_item_it) {
                    if (MissingReferencedItem(booster_item_it->item_index)) {
                        cMyTrace trace(kVerifyPrettyFunction,
                                       10074,
                                       9,
                                       true,
                                       true);
                        trace("Booster item element not found (%d)->(%d)",
                              item->get_index(),
                              booster_item_it->item_index);
                        item_verification_failed = true;
                    }
                }
            }
        } else if (item->vtable_->get_item_type(item) == 14) {
            const std::vector<STStackablePackageData>& package_items =
                stackable_item->package_data();
            for (std::vector<STStackablePackageData>::const_iterator it =
                     package_items.begin();
                 it != package_items.end(); ++it) {
                if (MissingReferencedItem(it->item_index)) {
                    cMyTrace trace(kVerifyPrettyFunction,
                                   10091,
                                   9,
                                   true,
                                   true);
                    trace("Package item element not found (%d)->(%d)",
                          item->get_index(),
                          it->item_index);
                    item_verification_failed = true;
                }
            }

            const std::vector<std::vector<STStackablePackageData> >&
                package_selections = stackable_item->package_selection_data();
            for (std::vector<std::vector<STStackablePackageData> >::
                     const_iterator selection_it = package_selections.begin();
                 selection_it != package_selections.end(); ++selection_it) {
                // The old implementation makes a vector copy for each
                // selection before visiting its members.
                const std::vector<STStackablePackageData> selected_items(
                    *selection_it);
                for (std::vector<STStackablePackageData>::const_iterator it =
                         selected_items.begin();
                     it != selected_items.end(); ++it) {
                    if (MissingReferencedItem(it->item_index)) {
                        cMyTrace trace(kVerifyPrettyFunction,
                                       10107,
                                       9,
                                       true,
                                       true);
                        trace("Package item selection element not found "
                              "(%d)->(%d)",
                              item->get_index(),
                              it->item_index);
                        item_verification_failed = true;
                    }
                }
            }
        } else {
            // Match the old final virtual call even though its return is
            // discarded in this branch.
            (void)item->vtable_->get_item_type(item);
        }
    }
    if (item_verification_failed) {
        return false;
    }

    // The original code has a flag for this pass, but each bad reference
    // writes zero back to it.  Keep the resulting log-only behavior exactly.
    bool quest_verification_failed = false;
    const __gnu_cxx::hash_map<int, Quest*>* const quests =
        quest_list_->getQuestList();
    for (__gnu_cxx::hash_map<int, Quest*>::const_iterator quest_it =
             quests->begin();
         quest_it != quests->end(); ++quest_it) {
        Quest* const quest = quest_it->second;
        if (quest->reward_type != 0 && quest->reward_type != 25) {
            continue;
        }

        for (std::vector<RewardItemInfo>::const_iterator it =
                 quest->reward_items.begin();
             it != quest->reward_items.end(); ++it) {
            const int item_id = it->item_id;
            if (static_cast<unsigned int>(item_id) > 2u && item_id != 999 &&
                !find_item(item_id)) {
                cMyTrace trace(kVerifyPrettyFunction, 10145, 5);
                trace("Quest reward item element not found "
                      "(quest: %d)(item: %d)",
                      quest->quest_index,
                      item_id);
                quest_verification_failed = false;
            }
        }
        for (std::vector<RewardItemInfo>::const_iterator it =
                 quest->light_reward_items.begin();
             it != quest->light_reward_items.end(); ++it) {
            const int item_id = it->item_id;
            if (static_cast<unsigned int>(item_id) > 2u && item_id != 999 &&
                !find_item(item_id)) {
                cMyTrace trace(kVerifyPrettyFunction, 10156, 5);
                trace("Quest reward select item element not found "
                      "(quest: %d)(item: %d)",
                      quest->quest_index,
                      item_id);
                quest_verification_failed = false;
            }
        }
        for (std::vector<int>::const_iterator it =
                 quest->clear_reward_items.begin();
             it != quest->clear_reward_items.end(); ++it) {
            if (!find_quest(*it)) {
                cMyTrace trace(kVerifyPrettyFunction, 10189, 5);
                trace("Quest reward collision quest element now found "
                      "(quest: %d)(collision quest: %d)",
                      quest->quest_index,
                      *it);
                quest_verification_failed = false;
            }
        }
        for (std::vector<RewardItemInfo>::const_iterator it =
                 quest->reward_select_items.begin();
             it != quest->reward_select_items.end(); ++it) {
            const int item_id = it->item_id;
            if (static_cast<unsigned int>(item_id) > 2u && item_id != 999 &&
                !find_item(item_id)) {
                cMyTrace trace(kVerifyPrettyFunction, 10202, 5);
                trace("Quest reward collision item element not found "
                      "(quest: %d)(item: %d)",
                      quest->quest_index,
                      item_id);
                quest_verification_failed = false;
            }
        }
    }
    if (quest_verification_failed) {
        return false;
    }

    for (__gnu_cxx::hash_map<int, CMonster*>::const_iterator it =
             monster_manager_->monsters_.begin();
         it != monster_manager_->monsters_.end(); ++it) {
        CMonster* const monster = it->second;
        if (monster->raw_58_ <= 0 || monster->raw_58_ > 200 ||
            monster->raw_5c_ <= 0 || monster->raw_5c_ > 200) {
            cMyTrace trace(kVerifyPrettyFunction, 10248, 9, true, true);
            trace("Error: Invalid monster(%d) level(%d-%d)",
                  monster->get_index(),
                  monster->raw_58_,
                  monster->raw_5c_);
        }
    }

    WongWork::CDungeonClear simulated_clear_data;
    for (CDungeonList::const_iterator it = dungeon_index_list_->begin();
         it != dungeon_index_list_->end(); ++it) {
        CDungeon* const dungeon = it->second;
        simulated_clear_data.addClearedDungeon(
            static_cast<short>(dungeon->get_index()), 4);
    }

    char serialized_clear_data[0x1800];
    if (simulated_clear_data.toString(serialized_clear_data,
                                      sizeof(serialized_clear_data)) < 0) {
        cMyTrace trace(kVerifyPrettyFunction, 10261, 9, true, true);
        trace("Simulate dungeon clear data fail!!!!");
        return false;
    }
    return true;
}

// IDA_EA: 0x8363CE0
// IDA_NAME: _ZN12CDataManager26reselectDailyTrainingQuestEv
// DEMANGLED: CDataManager::reselectDailyTrainingQuest(void)
bool CDataManager::reselectDailyTrainingQuest() {
    TrainingQuestScript* const training_quests = get_training_quest_script();
    (void)training_quests->suffleTrainingQuests();

    std::multiset<int>
        apply_levels[oldsrv::config::kTrainingQuestLevelCount];
    (void)training_quests->getApplyLevel(apply_levels);
    for (int level = oldsrv::config::kCharacterLevelMinimum;
         level <= oldsrv::config::kTrainingQuestLevelMaximum;
         ++level) {
        std::vector<int>* const daily_quests = getDailyTrainingQuest(level);
        daily_quests->clear();

        int previous_apply_level = INT_MAX;
        const std::multiset<int>& level_apply_levels =
            apply_levels[level - 1];
        for (std::multiset<int>::const_iterator it =
                 level_apply_levels.begin();
             it != level_apply_levels.end(); ++it) {
            const int apply_level = *it;
            const int quest_index = training_quests->getTrainingQuest(
                level, apply_level, previous_apply_level);
            previous_apply_level = apply_level;

            if (quest_index == INT_MAX) {
                continue;
            }

            Quest* const quest = quest_list_->find_quest(quest_index);
            if (quest && quest->quest_grade == 1) {
                daily_quests->push_back(quest_index);
            } else if (quest) {
                LogManager::logFormat(
                    1,
                    "data_manager.cpp",
                    kReselectDailyTrainingQuestPrettyFunction,
                    10952,
                    "reselectDailyTrainingQuest() ERROR : quest(%d), "
                    "level(%d)",
                    quest_index,
                    level);
            }
        }

        (void)training_quests->reset_same_apply_level_cnt();
        if (!daily_quests->empty()) {
            sync_script::CStringMaker input_maker;
            sync_script::CStringMaker rendered_quests = std::for_each(
                daily_quests->begin(), daily_quests->end(), input_maker);
            cMyTrace trace(kReselectDailyTrainingQuestPrettyFunction,
                           10962,
                           0);
            trace("TRAINING_QUEST level(%d), quest(%s)",
                  level,
                  rendered_quests.c_str());
        }
    }
    return true;
}
