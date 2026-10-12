#include "old_game/player/core/character_growth_tables.h"

#include "common/config.h"
#include "old_game/player/core/character_script_growth_data.h"

#include <cstring>


typedef char STSecondGrowTypeInfo_Size_Must_Be_0xc0[
    (sizeof(STSecondGrowTypeInfo) == 0xc0) ? 1 : -1];
typedef char CCharacter_Size_Must_Be_0x7dc[
    (sizeof(CCharacter) == 0x7dc) ? 1 : -1];
typedef char CCharacList_Size_Must_Be_0x5e50[
    (sizeof(CCharacList) == 0x5e50) ? 1 : -1];

namespace {

// CCharacter::set_data copies the 0x52-byte public-stat prefix of each
// STRDCharacterStatus. The trailing raw_52/raw_53 fields are not part of
// _Additioal_info: raw_52 is copied separately into CCharacter's flag bytes,
// while raw_53 is deliberately not consumed by the original routine.
void CopyAdditionalInfo(_Additioal_info& destination,
                        const STRDCharacterStatus& source) {
    destination.max_hp = source.raw_00;
    destination.max_mp = source.raw_04;
    destination.physical_attack = source.raw_08;
    destination.physical_defense = source.raw_0a;
    destination.magical_attack = source.raw_0c;
    destination.magical_defense = source.raw_0e;
    for (int index = 0; index <= 3; ++index) {
        destination.element_resist[index] =
            static_cast<uint16_t>(source.four_values[index]);
    }
    for (int index = 0; index <= 16; ++index) {
        destination.spec_property[index] =
            static_cast<uint16_t>(source.seventeen_values[index]);
    }
    destination.inventory_weight = static_cast<int32_t>(source.raw_3a);
    destination.hp_regen_speed = source.raw_3e;
    destination.mp_regen_speed = source.raw_40;
    destination.move_speed = source.raw_42;
    destination.attack_speed = source.raw_46;
    destination.cast_speed = source.raw_48;
    destination.hit_recovery = source.raw_4a;
    destination.jump_power = source.raw_4c;
    destination.character_weight = static_cast<int32_t>(source.raw_4e);
}

}  // namespace

// IDA_EA: 0x8373934
// IDA_NAME: _ZN20STSecondGrowTypeInfoC2Ev
// DEMANGLED: STSecondGrowTypeInfo::STSecondGrowTypeInfo(void)
STSecondGrowTypeInfo::STSecondGrowTypeInfo()
    : status(),
      skill_values() {
    clear();
}

// IDA_EA: 0x8376324
// IDA_NAME: _ZN20STSecondGrowTypeInfoD2Ev
// DEMANGLED: STSecondGrowTypeInfo::~STSecondGrowTypeInfo()
STSecondGrowTypeInfo::~STSecondGrowTypeInfo() {}

// IDA_EA: 0x8373A1E
// IDA_NAME: _ZN20STSecondGrowTypeInfo5clearEv
// DEMANGLED: STSecondGrowTypeInfo::clear(void)
void* STSecondGrowTypeInfo::clear() {
    for (int index = 0; index <= 1; ++index) {
        status[index].clear();
        skill_values[index].clear();
    }
    return std::memset(grow_flags, 0, sizeof(grow_flags));
}

// IDA_EA: 0x834828C
// IDA_NAME: _ZN10CCharacterC2Ev
// DEMANGLED: CCharacter::CCharacter(void)
CCharacter::CCharacter()
    : base_status_(),
      first_grow_skills_(),
      second_grow_infos_() {
    clear();
}

// IDA_EA: 0x834842A
// IDA_NAME: _ZN10CCharacterD2Ev
// DEMANGLED: CCharacter::~CCharacter()
CCharacter::~CCharacter() {
    destroy();
}

// IDA_EA: 0x834852E
// IDA_NAME: _ZN10CCharacter5clearEv
// DEMANGLED: CCharacter::clear(void)
bool CCharacter::clear() {
    base_status_[0].clear();
    for (int index = 0; index <= 5; ++index) {
        base_status_[index + 1].clear();
    }

    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 6;
        if (index > 6) {
            break;
        }

        first_grow_skills_[index].clear();
        second_grow_infos_[index].clear();
    }

    start_town_ = 0;

    return result;
}

// IDA_EA: 0x83485C4
// IDA_NAME: _ZN10CCharacter7destroyEv
// DEMANGLED: CCharacter::destroy(void)
bool CCharacter::destroy() {
    return clear();
}

// IDA_EA: 0x83485D8
// IDA_NAME: _ZNK10CCharacter28get_give_skill_firstGrowtypeEiRSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CCharacter::get_give_skill_firstGrowtype(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)const
int CCharacter::get_give_skill_firstGrowtype(
    int charac_job,
    std::vector<std::pair<int, int> >& skill_values) const {
    if (charac_job < 0 || charac_job > 5) {
        return 0;
    }

    const std::vector<std::pair<int, int> >& source =
        first_grow_skills_[charac_job + 1u];
    for (std::vector<std::pair<int, int> >::const_iterator iter =
             source.begin();
         iter != source.end(); ++iter) {
        skill_values.push_back(*iter);
    }
    return 1;
}

// IDA_EA: 0x8348694
// IDA_NAME: _ZNK10CCharacter29get_give_skill_secondGrowtypeEiiRSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CCharacter::get_give_skill_secondGrowtype(int,int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)const
int CCharacter::get_give_skill_secondGrowtype(
    int charac_job, int second_grow_type,
    std::vector<std::pair<int, int> >& skill_values) const {
    if (charac_job < 0 || charac_job > 5) {
        return 0;
    }
    if (second_grow_type <= 0 || second_grow_type > 2) {
        return 1;
    }

    const std::vector<std::pair<int, int> >& source =
        second_grow_infos_[charac_job + 1u]
            .skill_values[second_grow_type - 1];
    for (std::vector<std::pair<int, int> >::const_iterator iter =
             source.begin(); iter != source.end(); ++iter) {
        skill_values.push_back(*iter);
    }
    return 1;
}

// IDA_EA: 0x8348798
// IDA_NAME: _ZNK10CCharacter14get_give_skillEiiRSt6vectorISt4pairIiiESaIS2_EEj
// DEMANGLED: CCharacter::get_give_skill(int,int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,unsigned int)const
int CCharacter::get_give_skill(
    int charac_job, int second_grow_type,
    std::vector<std::pair<int, int> >& skill_values,
    unsigned int grow_stage) const {
    if (grow_stage != 0u) {
        if (grow_stage == 1u) {
            return get_give_skill_firstGrowtype(charac_job, skill_values);
        }
        if (grow_stage == 2u) {
            return get_give_skill_secondGrowtype(charac_job, second_grow_type,
                                                 skill_values);
        }
        return 0;
    }

    get_give_skill_firstGrowtype(charac_job, skill_values);
    // Full-tree rebuild/reset paths need every attained awakening stage.
    // Stage-specific transition callers still use grow_stage=2 and receive
    // only the newly attained stage, avoiding duplicate transition packets.
    for (int stage = 1; stage <= second_grow_type; ++stage) {
        if (get_give_skill_secondGrowtype(charac_job, stage,
                                          skill_values) != 1) return 0;
    }
    return 1;
}

// IDA_EA: 0x834883E
// IDA_NAME: _ZNK10CCharacter17get_growtype_statEii
// DEMANGLED: CCharacter::get_growtype_stat(int,int)const
_Additioal_info* CCharacter::get_growtype_stat(
    int charac_job, int grow_type) const {
    if (charac_job > 5 || grow_type > 2) {
        return 0;
    }
    if (grow_type != 0) {
        return const_cast<_Additioal_info*>(
            &second_grow_infos_[charac_job].status[grow_type - 1]);
    }
    return const_cast<_Additioal_info*>(&base_status_[charac_job + 1]);
}

// IDA_EA: 0x8348894
// IDA_NAME: _ZN10CCharacter8set_dataEP17STCharacterScript
// DEMANGLED: CCharacter::set_data(STCharacterScript *)
bool CCharacter::set_data(STCharacterScript* script) {
    for (int n6 = 0; n6 <= 6; ++n6) {
        CopyAdditionalInfo(base_status_[n6], script->growtype_status_[n6]);
        script_status_flags_[n6] = script->growtype_status_[n6].raw_52;
        first_grow_skills_[n6] = script->give_skills_[n6];
    }
    start_town_ = script->start_town_ > 0 && script->start_town_ < 256
                      ? static_cast<uint8_t>(script->start_town_)
                      : 0;

    bool result = false;
    for (int n6_1 = 0;; ++n6_1) {
        result = n6_1 <= 6;
        if (n6_1 > 6) {
            break;
        }
        for (int i = 0; i <= 1; ++i) {
            CopyAdditionalInfo(second_grow_infos_[n6_1].status[i],
                               script->second_grow_type_data_[n6_1].status[i]);
            second_grow_infos_[n6_1].skill_values[i] =
                script->second_grow_type_data_[n6_1].skill_values[i];
        }
    }
    return result;
}

// IDA_EA: 0x83493EA
// IDA_NAME: _ZN11CCharacListC2Ev
// DEMANGLED: CCharacList::CCharacList(void)
CCharacList::CCharacList()
    : characters_() {}

// IDA_EA: 0x834945E
// IDA_NAME: _ZN11CCharacListD2Ev
// DEMANGLED: CCharacList::~CCharacList()
CCharacList::~CCharacList() {
    destroy();
}

// IDA_EA: 0x83494DA
// IDA_NAME: _ZN11CCharacList7destroyEv
// DEMANGLED: CCharacList::destroy(void)
bool CCharacList::destroy() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index < oldsrv::config::kCharacterJobCount;
        if (index >= oldsrv::config::kCharacterJobCount) {
            break;
        }
        characters_[index].destroy();
    }
    return result;
}
