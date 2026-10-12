#include "old_game/progression/career/disjoint_abi.h"

#include "common/config.h"
#include "common/manager/find_price_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

#include <stdio.h>
#include <stdint.h>

// IDA_EA: 0x84735D2
// IDA_NAME: _ZN19DisjointSkillScript5clearEv
// DEMANGLED: DisjointSkillScript::clear(void)
DisjointSkillScript* DisjointSkillScript::clear() {
    character_job = oldsrv::config::kCharacterJobCommon;
    skill_index = 0;
    skill_level = 0;
    element = 4;
    return this;
}

// IDA_EA: 0x84735FE
// IDA_NAME: _ZN19DisjointSkillScriptC2Ev
// DEMANGLED: DisjointSkillScript::DisjointSkillScript(void)
DisjointSkillScript::DisjointSkillScript() {
    clear();
}

// IDA_EA: 0x88E5B32
// IDA_NAME: _ZN19DisjointSkillScriptD2Ev
// DEMANGLED: DisjointSkillScript::~DisjointSkillScript()
DisjointSkillScript::~DisjointSkillScript() {}

// IDA_EA: 0x82347A2
// IDA_NAME: _ZN14DisJointResultC2Ev
// DEMANGLED: DisJointResult::DisJointResult(void)
DisJointResult::DisJointResult() {
    for (int index = 0; index <= 4; ++index) {
        values[index] = 0;
        values[index + 5] = 0;
    }
}

// IDA_EA: 0x8375BCA
// IDA_NAME: _ZN14DisjointScript5clearEv
// DEMANGLED: DisjointScript::clear(void)
bool DisjointScript::clear() {
    for (int index = 0; index <= 4; ++index) {
        cube_indexes[index].clear();
    }
    for (int index = 0; index <= 3; ++index) {
        legacy_infos[index].clear();
    }
    skill_info_count = -1;
    skill_infos.clear();
    for (int index = 0; index <= 1; ++index) {
        skill_prob[index] = 0.0f;
    }
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        for (int grade = 0; grade <= 3; ++grade) {
            for (int element_index = 0; element_index <= 4; ++element_index) {
                avatar_disjoint_info[job][grade][element_index] = 0;
            }
        }
    }
    for (int grade = 0; grade <= 3; ++grade) {
        for (int part = 0; part <= 9; ++part) {
            avatar_parts_disjoint_info[grade][part] = 0;
        }
    }
    return false;
}

// IDA_EA: 0x8375D4C
// IDA_NAME: _ZN14DisjointScriptC2Ev
// DEMANGLED: DisjointScript::DisjointScript(void)
DisjointScript::DisjointScript() {
    clear();
}

// IDA_EA: 0x837DFF8
// IDA_NAME: _ZN14DisjointScriptD2Ev
// DEMANGLED: DisjointScript::~DisjointScript()
DisjointScript::~DisjointScript() {}

// IDA_EA: 0x837E532
// IDA_NAME: _ZN8DisJointC2Ev
// DEMANGLED: DisJoint::DisJoint(void)
DisJoint::DisJoint() {}

// IDA_EA: 0x837E11C
// IDA_NAME: _ZN8DisJointD2Ev
// DEMANGLED: DisJoint::~DisJoint()
DisJoint::~DisJoint() {}

// IDA_EA: 0x847300C
// IDA_NAME: _ZN8DisJoint12LoadDisJointEv
// DEMANGLED: DisJoint::LoadDisJoint(void)
int DisJoint::LoadDisJoint() {
    if (importDisjointScript(this, "Etc/Disjoint.etc") == 1) {
        for (int rarity = 0; rarity <= 5; ++rarity) {
            int value = cube_indexes[4].at(0);
            predict_results[rarity].push_back(value);
            for (int index = 0; additional_results[rarity].count > index;
                 ++index) {
                predict_results[rarity].push_back(
                    additional_results[rarity].items[index]);
            }
        }
        return 1;
    }

    ::puts("DisJoint Script Error: Etc/Disjoint.etc");
    return 0;
}

// IDA_EA: 0x8473116
// IDA_NAME: _ZN8DisJoint16GetPredictResultEi
// DEMANGLED: DisJoint::GetPredictResult(int)
std::vector<int32_t>* DisJoint::GetPredictResult(int rarity) {
    if (static_cast<unsigned int>(rarity) < 6u) {
        return &predict_results[rarity];
    }
    return &predict_results[0];
}

// IDA_EA: 0x8473148
// IDA_NAME: _ZNK8DisJoint29getPredictResultItemTypeCountEi
// DEMANGLED: DisJoint::getPredictResultItemTypeCount(int)const
int DisJoint::getPredictResultItemTypeCount(int rarity) const {
    if (static_cast<unsigned int>(rarity) >= 6u) {
        return 1;
    }
    if (additional_results[rarity].count != 0) {
        return 2;
    }
    return 1;
}

// IDA_EA: 0x8473184
// IDA_NAME: _ZN8DisJoint17GetAddtionalSkillEiiRiS0_
// DEMANGLED: DisJoint::GetAddtionalSkill(int,int,int &,int &)
int* DisJoint::GetAddtionalSkill(int character_job_value,
                                 int skill_index_value,
                                 int& skill_level_value,
                                 int& element_value) {
    DisjointSkillScript script;
    for (std::vector<DisjointSkillScript>::size_type index = 0;
         index < skill_infos.size(); ++index) {
        script = skill_infos[index];
        if (script.character_job == character_job_value &&
            script.skill_index == skill_index_value) {
            skill_level_value = script.skill_level;
            element_value = script.element;
            return &element_value;
        }
    }
    skill_level_value = 0;
    element_value = 0;
    return &element_value;
}

// IDA_EA: 0x8473224
// IDA_NAME: _ZN8DisJoint22_GetAdditionalItemInfoEi
// DEMANGLED: DisJoint::_GetAdditionalItemInfo(int)
int DisJoint::_GetAdditionalItemInfo(int rarity) {
    if (additional_results[rarity].count == 0) {
        return 0;
    }

    int roll = static_cast<int>(server_utility::get_rand_int(
        1000 * additional_results[rarity].count));
    return additional_results[rarity].items[roll / 1000];
}

// IDA_EA: 0x84732B4
// IDA_NAME: _ZN8DisJoint18_GetAdditionalItemEPK5CItemRiS3_
// DEMANGLED: DisJoint::_GetAdditionalItem(CItem const*,int &,int &)
char DisJoint::_GetAdditionalItem(const CItem* item,
                                  int& item_index,
                                  int& grade) {
    const CItem* public_item =
        reinterpret_cast<const CItem*>(item);
    int rarity = public_item->get_rarity();
    item_index = _GetAdditionalItemInfo(rarity);
    int result = item_index != 0;
    if (item_index != 0) {
        int threshold = 100 * additional_result_const[3 * rarity];
        if (static_cast<int>(server_utility::get_rand_int(10000)) >=
            threshold) {
            float item_grade = static_cast<float>(public_item->get_grade());
            grade = static_cast<int>(item_grade /
                                     additional_result_grade_b[rarity]);
        } else {
            float item_grade = static_cast<float>(public_item->get_grade());
            grade = static_cast<int>(item_grade /
                                     additional_result_grade_a[rarity]);
        }
        result = grade;
        if (grade <= 0) {
            result = static_cast<int>(reinterpret_cast<uintptr_t>(&grade));
            grade = 1;
        }
    }
    return static_cast<char>(result);
}

// IDA_EA: 0x84733F6
// IDA_NAME: _ZN8DisJoint9GetResultEiR14DisJointResult
// DEMANGLED: DisJoint::GetResult(int,DisJointResult &)
int DisJoint::GetResult(int item_index, DisJointResult& result) {
    CDataManager* data_manager = G_CDataManager();
    CItem* item = data_manager->find_item(item_index);
    if (item != 0) {
        float creation_rate = cube_creation_rates[item->get_rarity()];
        result.values[5] = cube_indexes[4][0];
        int sell_price = static_cast<int>(
            static_cast<long double>(item->GetSellPrice()) * 1.1L);

        typedef int (*ItemVtableFlag)(CItem*);
        void** vtable = *reinterpret_cast<void***>(item);
        int special_price_type =
            reinterpret_cast<ItemVtableFlag>(vtable[19])(item);

        int base_price = 1000 *
                         getItemSellPrice(sell_price,
                                          0,
                                          0,
                                          special_price_type != 0) /
                         cube_creation_const;
        result.values[0] = static_cast<int>(
            static_cast<long double>(base_price) * creation_rate / 1000.0L);
        if (result.values[0] <= 0) {
            result.values[0] = 1;
        }
        _GetAdditionalItem(reinterpret_cast<const CItem*>(item),
                           result.values[6],
                           result.values[1]);
        return 0;
    }

    LogManager::logFormat(
        1,
        "DisJoint.cpp",
        "int DisJoint::GetResult(int, DisJointResult&)",
        110,
        "[DisJoint::GetResult] Null Item\n");
    return 17;
}
