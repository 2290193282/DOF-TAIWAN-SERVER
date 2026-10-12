#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x830E9BA
// IDA_NAME: _ZN20DungeonRidableObjectC2Ev
// DEMANGLED: DungeonRidableObject::DungeonRidableObject(void)
DungeonRidableObject::DungeonRidableObject()
    : RidableObject(), state(0) {}

// IDA_EA: 0x830E9D8
// IDA_NAME: _ZN20DungeonRidableObjectaSE13RidableObject
// DEMANGLED: DungeonRidableObject::operator=(RidableObject)
DungeonRidableObject& DungeonRidableObject::operator=(RidableObject source) {
    value_00 = source.value_00;
    value_04 = source.value_04;
    value_08 = source.value_08;
    value_0c = source.value_0c;
    value_10 = source.value_10;
    return *this;
}

namespace {

void InsertRidableObject(CRidable* ridable,
                         const DungeonRidableObject& object) {
    ridable->insert(std::make_pair(object.value_08, object));
}

}  // namespace

// IDA_EA: 0x830A862
// IDA_NAME: _ZN13CBattle_Field32check_random_appear_hell_dungeonEPK8CDungeon17ENUM_DUNGEON_MODE17ENUM_DUNGEON_TYPERb
// DEMANGLED: CBattle_Field::check_random_appear_hell_dungeon(CDungeon const*,ENUM_DUNGEON_MODE,ENUM_DUNGEON_TYPE,bool &)
CParty* CBattle_Field::check_random_appear_hell_dungeon(
    const CDungeon* dungeon,
    ENUM_DUNGEON_MODE dungeon_mode,
    ENUM_DUNGEON_TYPE dungeon_type,
    bool& should_appear) {
    CParty* const party = party_;
    if (!party) {
        return 0;
    }

    if (party->world_map_ && IsEnableHellDungeon() &&
        party->world_map_->IsInHellDungeon() == 1) {
        int maximum_fatigue = 1;
        int minimum_fatigue = 200;
        int valid_members = 0;
        for (int seat = 0; seat <= 3; ++seat) {
            if (party->checkValidUser(seat) != 1) {
                continue;
            }

            CUser* const user = party->get_user(seat);
            const _Charac_info* const character =
                user ? user->getCurCharacR() : 0;
            const int fatigue = character
                ? static_cast<int>(character->fields_.fatigue_)
                : 1;
            maximum_fatigue = std::max(maximum_fatigue, fatigue);
            minimum_fatigue = std::min(minimum_fatigue, fatigue);
            ++valid_members;
        }

        if (maximum_fatigue - dungeon->get_standard_level() <= 5 &&
            (valid_members <= 1 || maximum_fatigue - minimum_fatigue <= 8)) {
            should_appear = true;
        }

        if ((maximum_fatigue - minimum_fatigue > 8 && valid_members > 1) ||
            (party->hell_quest_notice_ != 1 &&
             static_cast<int>(dungeon_type) == 1) ||
            (party->hell_dungeon_seal_door_ready_ != 1 &&
             static_cast<int>(dungeon_type) == 1)) {
            party->world_map_ = 0;
        }
    }

    if (static_cast<int>(dungeon_mode) == 1) {
        party->world_map_ = 0;
        should_appear = false;
    }
    return party;
}

// IDA_EA: 0x85B6478
// IDA_NAME: _ZN6CParty20CheckQuestConnectionEi20ENUM_MAZE_QUEST_TYPE
// DEMANGLED: CParty::CheckQuestConnection(int,ENUM_MAZE_QUEST_TYPE)
int CParty::CheckQuestConnection(int quest_index,
                                 ENUM_MAZE_QUEST_TYPE quest_type) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (!static_cast<unsigned char>(_checkValidUser(seat))) {
            continue;
        }

        CUser* const user = get_user(seat);
        if (!user) {
            continue;
        }
        UserQuest* const quests = user->getCurCharacQuestR();
        if (!quests) {
            continue;
        }

        if (static_cast<int>(quest_type) == 0) {
            if (static_cast<unsigned char>(
                    quests->IsDoingQuest(quest_index))) {
                return 1;
            }
        } else if (static_cast<int>(quest_type) == 1 &&
                   static_cast<unsigned char>(
                       quests->isClearQuest(quest_index))) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x859BBA8
// IDA_NAME: _ZN6CParty25increase_hell_party_ratioEPc
// DEMANGLED: CParty::increase_hell_party_ratio(char *)
bool CParty::increase_hell_party_ratio(char* weights) {
    bool result = getPartyMemberAvgLevel() <= 59;
    if (!result) {
        QuickParty::RewardData_Param parameter(this);
        parameter.raw_04 =
            battle_field().get_dungeon_index();
        QuickParty::RewardData_Result reward_result;
        reward_result.raw_00 = weights[1];
        reward_result.raw_04 = weights[2];
        set_basic_reward(
            static_cast<QuickParty::BasicRewardType>(12),
            parameter,
            reward_result);
        weights[1] = static_cast<char>(reward_result.raw_00);
        result = reward_result.raw_04;
        weights[2] = static_cast<char>(reward_result.raw_04);
    }
    return result;
}

// IDA_EA: 0x82FF128
// IDA_NAME: _ZNK13CBattle_Field18GetAppropriateMazeE23ENUM_DUNGEON_DIFFICULTY
// DEMANGLED: CBattle_Field::GetAppropriateMaze(ENUM_DUNGEON_DIFFICULTY) const
MazeScript* CBattle_Field::GetAppropriateMaze(
    ENUM_DUNGEON_DIFFICULTY difficulty) const {
    CParty* const party = party_;
    CDungeon* const dungeon = dungeon_;
    const int difficulty_value = static_cast<int>(difficulty);

    for (int quest_type = 0; quest_type <= 1; ++quest_type) {
        const std::vector<MazeScript>* const mazes =
            dungeon->GetQuestMazeScriptVector(
                static_cast<ENUM_MAZE_QUEST_TYPE>(quest_type));
        if (!mazes || mazes->empty()) {
            continue;
        }

        std::vector<MazeScript*> candidates;
        for (std::vector<MazeScript>::const_iterator it = mazes->begin();
             it != mazes->end();
             ++it) {
            MazeScript* const maze = const_cast<MazeScript*>(&*it);
            if (instance_dungeon_mode_ == 1 ||
                (hell_party_enabled_ && maze->value_70) ||
                !party ||
                !static_cast<unsigned char>(
                    party->CheckQuestConnection(
                        maze->value_50,
                        static_cast<ENUM_MAZE_QUEST_TYPE>(quest_type)))) {
                continue;
            }

            Quest* const quest =
                G_CDataManager()->find_quest(maze->value_50);
            if (quest && quest->cant_giveup &&
                party->get_member_count() > 1) {
                continue;
            }

            if (quest_type != 0 ||
                difficulty_value >= maze->value_54) {
                candidates.push_back(maze);
            }
        }

        if (candidates.size() == 1u) {
            return candidates[0];
        }
        if (candidates.size() > 1u) {
            return candidates[
                static_cast<unsigned int>(std::rand()) %
                candidates.size()];
        }
    }

    return dungeon->GetRandMazebyPartyCount(party->get_member_count());
}

// IDA_EA: 0x82FF988
// IDA_NAME: _ZN13CBattle_Field23CheckHellPartyConditionEv
// DEMANGLED: CBattle_Field::CheckHellPartyCondition(void)
int CBattle_Field::CheckHellPartyCondition() {
    CParty* const party = party_;
    if (!party) {
        return 0;
    }

    const int party_value = static_cast<int>(
        party->battle_data().hell_party_value_total());
    if (party_value < static_cast<int>(get_rand_int(100))) {
        return 0;
    }

    const stHellParty_t* const script =
        G_CDataManager()->get_hellparty_script_values();
    return party_value >= script->guide_line_value;
}

// IDA_EA: 0x82FFA2E
// IDA_NAME: _ZN13CBattle_Field22GetHellPartyDifficultyE17ENUM_DUNGEON_TYPE
// DEMANGLED: CBattle_Field::GetHellPartyDifficulty(ENUM_DUNGEON_TYPE)
int CBattle_Field::GetHellPartyDifficulty(ENUM_DUNGEON_TYPE dungeon_type) {
    char weights[6] = {0, 0, 0, 0, 0, 0};
    const stHellParty_t* const script =
        G_CDataManager()->get_hellparty_script_values();
    if (static_cast<int>(dungeon_type) == 1) {
        std::memcpy(weights, script->difficulty_extra_84, sizeof(weights));
    } else {
        std::memcpy(weights, script->difficulty_extra_7e, sizeof(weights));
        if (party_) {
            party_->increase_hell_party_ratio(weights);
        }
    }

    int accumulated = 0;
    const int random_value = static_cast<int>(get_rand_int(100));
    for (int difficulty = 1; difficulty <= 5; ++difficulty) {
        accumulated += weights[difficulty];
        if (accumulated >= random_value) {
            return difficulty;
        }
    }
    return 0;
}

// IDA_EA: 0x82FFB06
// IDA_NAME: _ZN13CBattle_Field21GetHellPartyEventTypeEc
// DEMANGLED: CBattle_Field::GetHellPartyEventType(char)
int CBattle_Field::GetHellPartyEventType(char difficulty) {
    const stHellParty_t* const script =
        G_CDataManager()->get_hellparty_script_values();
    return static_cast<signed char>(
        script->difficulty_extra_8a[static_cast<unsigned char>(difficulty)]);
}

// IDA_EA: 0x830E170
// IDA_NAME: _ZN13CBattle_Field16setQuestMazeInfoEv
// DEMANGLED: CBattle_Field::setQuestMazeInfo(void)
bool CBattle_Field::setQuestMazeInfo() {
    CParty* const party = party_;
    CDungeon* const dungeon = dungeon_;
    for (int quest_type = 0; quest_type <= 1; ++quest_type) {
        const std::vector<MazeScript>* const mazes =
            dungeon->GetQuestMazeScriptVector(
                static_cast<ENUM_MAZE_QUEST_TYPE>(quest_type));
        if (!mazes || mazes->empty()) {
            continue;
        }

        for (std::vector<MazeScript>::const_iterator it = mazes->begin();
             it != mazes->end();
             ++it) {
            if (instance_dungeon_mode_ == 1 ||
                !party ||
                !static_cast<unsigned char>(
                    party->CheckQuestConnection(
                        it->value_50,
                        static_cast<ENUM_MAZE_QUEST_TYPE>(quest_type)))) {
                continue;
            }

            Quest* const quest =
                G_CDataManager()->find_quest(it->value_50);
            if (quest &&
                (!quest->cant_giveup ||
                 party->get_member_count() == 1)) {
                setQuestMaze(true);
            }
        }
    }
    return false;
}

// IDA_EA: 0x82FEF12
// IDA_NAME: _ZN15CClearCondition30SetDungeonClearConditionVectorERSt6vectorI21DungeonClearConditionSaIS1_EE
// DEMANGLED: CClearCondition::SetDungeonClearConditionVector(std::vector<DungeonClearCondition> &)
int CClearCondition::SetDungeonClearConditionVector(
    std::vector<DungeonClearCondition>& conditions) {
    Clear();
    if (conditions.empty()) {
        return 0;
    }

    condition_source_ = &conditions;
    for (std::vector<DungeonClearCondition>::const_iterator it =
             conditions.begin();
         it != conditions.end();
         ++it) {
        current_count_ += it->value_08;
        cleared_states_.push_back(0);
    }
    return 0;
}

// IDA_EA: 0x82FE6D0
// IDA_NAME: _ZN8CRidable16SetRidableObjectER13RidableScript
// DEMANGLED: CRidable::SetRidableObject(RidableScript &)
char CRidable::SetRidableObject(RidableScript& script) {
    if (script.value_00 <= 0) {
        return static_cast<char>(script.value_00);
    }

    reveal_pending_ = script.value_04;
    requested_object_count_ = script.value_00;

    // IDA 0x82FE6F1 constructs this single object before the branch.  The
    // random branch copies all six source words directly; the full-list
    // branch calls the five-word assignment operator and deliberately keeps
    // the constructor's value_14 (200).
    DungeonRidableObject object;

    if (script.value_00 <
        static_cast<int>(script.objects.size())) {
        std::vector<RidableObject> candidates(script.objects);
        for (int index = 0; index < script.value_00; ++index) {
            std::random_shuffle(candidates.begin(), candidates.end());
            const RidableObject selected = candidates.back();
            candidates.pop_back();
            object.value_00 = selected.value_00;
            object.value_04 = selected.value_04;
            object.value_08 = selected.value_08;
            object.value_0c = selected.value_0c;
            object.value_10 = selected.value_10;
            object.value_14 = selected.value_14;
            object.state = 4;
            InsertRidableObject(this, object);
        }
    } else {
        for (std::vector<RidableObject>::const_iterator it =
                 script.objects.begin();
             it != script.objects.end();
             ++it) {
            object = *it;
            object.state = 4;
            InsertRidableObject(this, object);
        }
    }
    return 0;
}
