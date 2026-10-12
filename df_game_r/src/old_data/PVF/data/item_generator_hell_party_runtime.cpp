#include "old_data/PVF/data/item_generator_drop_types.h"

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/inventory/inventory.h"

extern "C++" int roundFloat(float value);

namespace WongWork {

// IDA_EA: 0x853A60E
// IDA_NAME: _ZN8WongWork17CMonsterDrop_HellD2Ev
// DEMANGLED: WongWork::CMonsterDrop_Hell::~CMonsterDrop_Hell()
//
// The compiler-generated destruction order is the exact IDA order:
// item_rate_table_ (+0x254), scaled_probabilities_ (+0x24),
// probabilities_ (+0x18), then CItemGenerator.
CMonsterDrop_Hell::~CMonsterDrop_Hell() {}

// IDA_EA: 0x8535948
// IDA_NAME: _ZN8WongWork17CMonsterDrop_Hell12setDropRatioEf
// DEMANGLED: WongWork::CMonsterDrop_Hell::setDropRatio(float)
void CMonsterDrop_Hell::setDropRatio(float ratio) {
    scaled_probabilities_ = probabilities_;
    for (std::vector<stGenItemProb_t>::iterator probability =
             scaled_probabilities_.begin();
         probability != scaled_probabilities_.end();
         ++probability) {
        for (int rarity = 0; rarity <= 4; ++rarity) {
            probability->weight[rarity] = roundFloat(static_cast<float>(
                static_cast<long double>(probability->weight[rarity]) * ratio));
        }
    }
}

// IDA_EA: 0x8535A88
// IDA_NAME: _ZN8WongWork17CMonsterDrop_Hell18_getGenItemProbIdxEi
// DEMANGLED: WongWork::CMonsterDrop_Hell::_getGenItemProbIdx(int)
int CMonsterDrop_Hell::_getGenItemProbIdx(int standard_level) {
    for (size_t index = 0; index < scaled_probabilities_.size(); ++index) {
        const stGenItemProb_t& probability = scaled_probabilities_[index];
        if (probability.first_level <= standard_level &&
            probability.last_level >= standard_level) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x8535726
// IDA_NAME: _ZN8WongWork17CMonsterDrop_Hell20generateSpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CMonsterDrop_Hell::generateSpecificItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CMonsterDrop_Hell::generateSpecificItem(
    const stGenerateRefData_t& reference,
    stGenerateResult_t& result) {
    const stGenerateRefMonsterSpecificDrop_t& specific =
        reference.generator_data.monster_specific_drop;
    const int probability_index =
        _getGenItemProbIdx(static_cast<int>(specific.dungeon_standard_level));
    if (probability_index < 0) {
        return;
    }

    const stGenItemProb_t& probability =
        scaled_probabilities_[static_cast<size_t>(probability_index)];
    const int drop_value = reference.standard_drop_rate *
        probability.weight[specific.dungeon_difficulty] / 100;
    if (random_number_.generateNumber(0, 1000) > drop_value) {
        return;
    }

    const int random_number = random_number_.generateNumber(1, 1000000);
    int* const rarity_decision = &rarity_decision_[0][0] +
        6 * static_cast<int>(specific.hell_rarity_decision_index);
    const int rarity = result.luck_point.GetItemRarity(
        rarity_decision, random_number, 2);
    const int item_id = item_rate_table_.chooseItem(
        2, rarity, static_cast<int>(specific.dungeon_standard_level));
    if (item_id == -1) {
        return;
    }

    CItemList* const item_list =
        G_CDataManager()->get_arad_jumping_item_list();
    CItem* const item_prototype = item_list->find_item(item_id);
    if (!item_prototype) {
        return;
    }

    Inven_Item item;
    item.item_id_ = static_cast<uint32_t>(item_id);
    item_list->create_item(static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
    const int item_index = static_cast<int>(result.items.size());
    result.luck_point.UseLuckPoint(
        static_cast<int>(item.item_category_),
        item_prototype->get_rarity(),
        item_prototype->get_grade(),
        item_index);
    result.items.push_back(item);
}

// IDA_EA: 0x8535B08
// IDA_NAME: _ZN8WongWork17CMonsterDrop_Hell21hellPartySpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CMonsterDrop_Hell::hellPartySpecificItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CMonsterDrop_Hell::hellPartySpecificItem(
    const stGenerateRefData_t& reference,
    stGenerateResult_t& result) {
    const stGenerateRefMonsterSpecificDrop_t& specific =
        reference.generator_data.monster_specific_drop;
    int specific_item_count = 0;

    CAICharacter* const ai_character = G_CDataManager()
        ->get_ai_character_list()
        ->get(specific.monster_id);
    if (ai_character) {
        if (ai_character->IsHellMonster()) {
            specific_item_count = 0;
        } else {
            specific_item_count = static_cast<int>(specific.monster_drop_count);
        }
        ai_character->generateIndependentItem(reference, result.items);
        ai_character->generateDropAvatarItem(
            specific.dungeon_difficulty,
            specific.avatar_drop_context,
            result.items);
    }

    CMonster* const monster = G_CDataManager()->find_monster(
        static_cast<int>(specific.monster_id));
    if (monster) {
        if (monster->IsHellMonster()) {
            specific_item_count = 0;
        } else {
            specific_item_count = static_cast<int>(specific.monster_drop_count);
        }

        if (!monster->getMonsterScript()->independent_drops.empty()) {
            monster->generateIndependentItem(reference, result.items);
        }
        monster->generateDropAvatarItem(specific.dungeon_difficulty,
                                        specific.avatar_drop_context,
                                        result.items);
        if (!monster->getMonsterScript()->catch_items.empty() &&
            specific.catch_monster_drop_enabled) {
            monster->generateCatchMonsterItem(result.items);
        }
    }

    if (specific.hell_party_repeat_specific_items) {
        for (int index = 0; index < specific_item_count; ++index) {
            // The old body performs an indirect call through vptr + 0x0c.
            // Preserve that dispatch rather than substituting a direct call.
            CItemGenerator* const generator = this;
            generator->generateSpecificItem(reference, result);
        }
    }
}

}  // namespace WongWork
