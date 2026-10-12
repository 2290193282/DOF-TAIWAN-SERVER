#include "common/manager/user.h"
#include "common/config.h"
#include "old_game/progression/skill/create_character_skill_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/skill/skill_changer_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_checksum_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <stdint.h>
#include <vector>



namespace {

const int kSkillSlotCount = 204;
const int kSkillSlotBytes =
    static_cast<int>(sizeof(_Mastered_skill)) * kSkillSlotCount;
static_assert(sizeof(_Mastered_skill) * kSkillSlotCount == 0x198u,
              "0627 mastered-skill tree must stay 0x198 bytes");

}  // namespace

// IDA_EA: 0x8694D6A
// IDA_NAME: _ZN9SkillSlotC2Ev
// DEMANGLED: SkillSlot::SkillSlot(void)
SkillSlot::SkillSlot()
    : parent_(0),
      remain_sp_tree_0_(0),
      load_striker_skill_(false),
      remain_sp_tree_1_(0),
      selected_skill_tree_index_(-1),
      remain_sfp_tree_0_(0),
      remain_sfp_tree_1_(0),
      combo_skill_(),
      skill_tree_0_(),
      skill_tree_1_(),
      temporary_tree_() {}

// IDA_EA: 0x8694E80
// IDA_NAME: _ZN9SkillSlotD2Ev
// DEMANGLED: SkillSlot::~SkillSlot()
SkillSlot::~SkillSlot() {}

// IDA_EA: 0x86996F6
// IDA_NAME: _ZN9SkillSlotC2ERKS_
// DEMANGLED: SkillSlot::SkillSlot(SkillSlot const&)
SkillSlot::SkillSlot(const SkillSlot& other)
    : parent_(other.parent_),
      remain_sp_tree_0_(other.remain_sp_tree_0_),
      load_striker_skill_(other.load_striker_skill_),
      remain_sp_tree_1_(other.remain_sp_tree_1_),
      selected_skill_tree_index_(other.selected_skill_tree_index_),
      remain_sfp_tree_0_(other.remain_sfp_tree_0_),
      remain_sfp_tree_1_(other.remain_sfp_tree_1_),
      combo_skill_(other.combo_skill_),
      skill_tree_0_(),
      skill_tree_1_(),
      temporary_tree_(),
      temporary_remain_sp_(other.temporary_remain_sp_),
      temporary_remain_sfp_(other.temporary_remain_sfp_) {
    std::memcpy(skill_tree_0_, other.skill_tree_0_, sizeof(skill_tree_0_));
    std::memcpy(skill_tree_1_, other.skill_tree_1_, sizeof(skill_tree_1_));
    std::memcpy(temporary_tree_, other.temporary_tree_,
                sizeof(temporary_tree_));
}

// IDA_EA: 0x869D91E
// IDA_NAME: _ZN9SkillSlotaSERKS_
// DEMANGLED: SkillSlot::operator=(SkillSlot const&)
SkillSlot& SkillSlot::operator=(const SkillSlot& other) {
    if (this == &other) {
        return *this;
    }

    parent_ = other.parent_;
    remain_sp_tree_0_ = other.remain_sp_tree_0_;
    load_striker_skill_ = other.load_striker_skill_;
    remain_sp_tree_1_ = other.remain_sp_tree_1_;
    selected_skill_tree_index_ = other.selected_skill_tree_index_;
    remain_sfp_tree_0_ = other.remain_sfp_tree_0_;
    remain_sfp_tree_1_ = other.remain_sfp_tree_1_;
    combo_skill_ = other.combo_skill_;
    std::memcpy(skill_tree_0_, other.skill_tree_0_, sizeof(skill_tree_0_));
    std::memcpy(skill_tree_1_, other.skill_tree_1_, sizeof(skill_tree_1_));
    std::memcpy(temporary_tree_, other.temporary_tree_,
                sizeof(temporary_tree_));
    temporary_remain_sp_ = other.temporary_remain_sp_;
    temporary_remain_sfp_ = other.temporary_remain_sfp_;
    return *this;
}

// IDA_EA: 0x822EE2E
// IDA_NAME: _ZN9SkillSlot10set_parentEP5CUser
// DEMANGLED: SkillSlot::set_parent(CUser *)
SkillSlot* SkillSlot::set_parent(CUser* user) {
    parent_ = user;
    return this;
}

// IDA_EA: 0x8603378
// IDA_NAME: _ZN9SkillSlot14set_skill_slotEPcS0_i
// DEMANGLED: SkillSlot::set_skill_slot(char *,char *,int)
void* SkillSlot::set_skill_slot(char* src, char* src_1, int size) {
    if (size > kSkillSlotBytes) {
        size = kSkillSlotBytes;
    }
    std::memcpy(skill_tree_0_, src, static_cast<size_t>(size));
    return std::memcpy(skill_tree_1_, src_1, static_cast<size_t>(size));
}

// IDA_EA: 0x86033CC
// IDA_NAME: _ZNK9SkillSlot14get_skill_slotEPcS0_i
// DEMANGLED: SkillSlot::get_skill_slot(char *,char *,int)const
void* SkillSlot::get_skill_slot(char* dest, char* dest_1, int size) const {
    if (size > kSkillSlotBytes) {
        size = kSkillSlotBytes;
    }
    std::memcpy(dest, skill_tree_0_, static_cast<size_t>(size));
    return std::memcpy(dest_1, skill_tree_1_, static_cast<size_t>(size));
}

// IDA_EA: 0x8603420
// IDA_NAME: _ZN9SkillSlot23set_skill_slot_at_indexEPci20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::set_skill_slot_at_index(char *,int,ENUM_SKILL_TREE_KIND)
void* SkillSlot::set_skill_slot_at_index(char* src, int size,
                                         ENUM_SKILL_TREE_KIND kind) {
    if (size > kSkillSlotBytes) {
        size = kSkillSlotBytes;
    }
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        return std::memcpy(skill_tree_0_, src, static_cast<size_t>(size));
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return std::memcpy(skill_tree_1_, src, static_cast<size_t>(size));
    }
    return reinterpret_cast<void*>(static_cast<intptr_t>(kind));
}

// IDA_EA: 0x860348A
// IDA_NAME: _ZNK9SkillSlot23get_skill_slot_at_indexEPci20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::get_skill_slot_at_index(char *,int,ENUM_SKILL_TREE_KIND)const
void* SkillSlot::get_skill_slot_at_index(char* dest, int size,
                                         ENUM_SKILL_TREE_KIND kind) const {
    if (size > kSkillSlotBytes) {
        size = kSkillSlotBytes;
    }
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        return std::memcpy(dest, skill_tree_0_,
                           static_cast<size_t>(size));
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return std::memcpy(dest, skill_tree_1_,
                           static_cast<size_t>(size));
    }
    return reinterpret_cast<void*>(static_cast<intptr_t>(kind));
}

// IDA_EA: 0x86034F8
// IDA_NAME: _ZN9SkillSlot22set_remain_sp_at_indexEi20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::set_remain_sp_at_index(int,ENUM_SKILL_TREE_KIND)
int SkillSlot::set_remain_sp_at_index(int value, ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        remain_sp_tree_0_ = value;
        return reinterpret_cast<int>(this);
    }
    if (kind == ENUM_SKILL_TREE_1) {
        remain_sp_tree_1_ = value;
        return reinterpret_cast<int>(this);
    }
    return kind;
}

// IDA_EA: 0x8603528
// IDA_NAME: _ZNK9SkillSlot22get_remain_sp_at_indexE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::get_remain_sp_at_index(ENUM_SKILL_TREE_KIND)const
int SkillSlot::get_remain_sp_at_index(ENUM_SKILL_TREE_KIND kind) const {
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        return remain_sp_tree_0_;
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return remain_sp_tree_1_;
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "int SkillSlot::get_remain_sp_at_index(ENUM_SKILL_TREE_KIND) const",
        0x68, "Wrong Skill Tree Kind : %d", kind);
    return 0;
}

// IDA_EA: 0x8603590
// IDA_NAME: _ZN9SkillSlot23set_remain_sfp_at_indexEi20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::set_remain_sfp_at_index(int,ENUM_SKILL_TREE_KIND)
int SkillSlot::set_remain_sfp_at_index(int value,
                                       ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_SFP_0) {
        remain_sfp_tree_0_ = value;
        return reinterpret_cast<int>(this);
    }
    if (kind == ENUM_SKILL_TREE_SFP_1) {
        remain_sfp_tree_1_ = value;
        return reinterpret_cast<int>(this);
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "void SkillSlot::set_remain_sfp_at_index(int, ENUM_SKILL_TREE_KIND)",
        0x75, "Wrong Skill Tree Kind : %d", kind);
    return 0;
}

// IDA_EA: 0x86035F2
// IDA_NAME: _ZNK9SkillSlot23get_remain_sfp_at_indexE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::get_remain_sfp_at_index(ENUM_SKILL_TREE_KIND)const
int SkillSlot::get_remain_sfp_at_index(ENUM_SKILL_TREE_KIND kind) const {
    if (kind == ENUM_SKILL_TREE_SFP_0) {
        return remain_sfp_tree_0_;
    }
    if (kind == ENUM_SKILL_TREE_SFP_1) {
        return remain_sfp_tree_1_;
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "int SkillSlot::get_remain_sfp_at_index(ENUM_SKILL_TREE_KIND) const",
        0x81, "Wrong Skill Tree Kind : %d", kind);
    return 0;
}

// IDA_EA: 0x8603652
// IDA_NAME: _ZNK9SkillSlot15get_skill_countEv
// DEMANGLED: SkillSlot::get_skill_count(void)const
int SkillSlot::get_skill_count() const {
    CUser* user = parent_;
    if (!user) {
        return 0;
    }
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    const _Mastered_skill* const slots = get_skillslot_buf(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    if (!slots) {
        return 0;
    }
    int count = 0;
    for (int index = 0; index <= 203; ++index) {
        if (slots[index].skill_index) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x86036CE
// IDA_NAME: _ZNK9SkillSlot9FindSkillEi20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::FindSkill(int,ENUM_SKILL_TREE_KIND)const
int SkillSlot::FindSkill(int skill_index, ENUM_SKILL_TREE_KIND kind) const {
    const _Mastered_skill* const slots = get_skillslot_buf(kind);
    if (!slots) {
        return 0;
    }
    for (int index = 0; index <= 203; ++index) {
        if (slots[index].skill_index == skill_index) {
            return slots[index].skill_level;
        }
    }
    return 0;
}

// IDA_EA: 0x860373C
// IDA_NAME: _ZNK9SkillSlot9FindSkillEi
// DEMANGLED: SkillSlot::FindSkill(int)const
int SkillSlot::FindSkill(int skill_index) const {
    CUser* user = parent_;
    if (!user) {
        return 0;
    }
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    const _Mastered_skill* const slots = get_skillslot_buf(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    if (!slots) {
        return 0;
    }
    for (int index = 0; index <= 203; ++index) {
        if (slots[index].skill_index == skill_index) {
            // IDA @0x860373C deliberately reads +0x47 from tree 0 after
            // using the selected tree only for the identity comparison.
            return skill_tree_0_[index].skill_level;
        }
    }
    return 0;
}

// IDA_EA: 0x8604324
// IDA_NAME: _ZN9SkillSlot12insert_skillEiib20ENUM_SKILL_TREE_KINDi
// DEMANGLED: SkillSlot::insert_skill(int,int,bool,ENUM_SKILL_TREE_KIND,int)
int SkillSlot::insert_skill(int skill_index, int group, bool active,
                            ENUM_SKILL_TREE_KIND kind, int level) {
    if (!parent_) {
        return -1;
    }
    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    if (!slots) {
        return -1;
    }
    const int slot_no = get_skillslot_no(0, group, kind, active);
    if (slot_no != -1) {
        slots[slot_no].skill_index = static_cast<uint8_t>(skill_index);
        slots[slot_no].skill_level = static_cast<int8_t>(level);
    }
    return slot_no;
}

// IDA_EA: 0x86043B4
// IDA_NAME: _ZN9SkillSlot28insert_skill_with_slot_indexEiibi
// DEMANGLED: SkillSlot::insert_skill_with_slot_index(int,int,bool,int)
int SkillSlot::insert_skill_with_slot_index(int slot_index,
                                            int unused_group,
                                            bool unused_active,
                                            int unused_level) {
    (void)unused_group;
    (void)unused_active;
    (void)unused_level;

    if (slot_index < 0) {
        return -4;
    }

    // This is the established owner-local SkillSlot parent projection used by
    // set_parent() and GetSkillTreeIndex(); it is the raw function's first
    // null gate, not a new CUser storage bridge.
    if (!parent_) {
        return -1;
    }

    const ENUM_SKILL_TREE_KIND kind =
        static_cast<ENUM_SKILL_TREE_KIND>(GetSkillTreeIndex());
    _Mastered_skill* const skill_tree = get_mastered_skill_tree(kind);
    if (!skill_tree) {
        return -1;
    }
    skill_tree[slot_index].skill_index = static_cast<uint8_t>(slot_index);
    skill_tree[slot_index].skill_level = 1;
    return slot_index;
}

// IDA_EA: 0x86040BC
// IDA_NAME: _ZN9SkillSlot14growtype_skillEiii20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::growtype_skill(
//             int,int,int,ENUM_SKILL_TREE_KIND)
int SkillSlot::growtype_skill(int charac_job,
                              int skill_index,
                              int skill_level,
                              ENUM_SKILL_TREE_KIND kind) {
    CSkill* const skill =
        G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        LogManager::logFormat(
            1,
            "skill_slot.cpp",
            "int SkillSlot::growtype_skill(int, int, int, ENUM_SKILL_TREE_KIND)",
            0x20B,
            "G_CDataManager()->find_skill( %d, %d )",
            charac_job,
            skill_index);
        return -1;
    }

    CUser* const user = parent_;
    if (!user) {
        return -1;
    }

    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    if (!slots) {
        return -1;
    }

    const int group = skill->get_group();
    int slot_no = get_skillslot_no(
        skill_index, group, kind, true);
    if (!skill_level) {
        if (slot_no != -1) {
            slots[slot_no].skill_index = 0;
            slots[slot_no].skill_level = 0;
        }
        CUser_history_log(user)->SkillDel(
            user->get_charac_job(),
            user->GetCurCharacSkillTreeIndex(),
            skill_index,
            0,
            static_cast<eSkillDelReason>(0));
        return slot_no;
    }

    if (slot_no == -1) {
        slot_no = insert_skill(
            skill_index, group, skill->is_active_skill(), kind, 1);
        if (slot_no >= 0) {
            slots[slot_no].skill_level = static_cast<int8_t>(skill_level);
        }
    } else {
        slots[slot_no].skill_level = static_cast<int8_t>(skill_level);
    }

    CUser_history_log(user)->SkillAdd(
        user->get_charac_job(),
        user->GetCurCharacSkillTreeIndex(),
        skill_index,
        skill_level,
        static_cast<eSkillAddReason>(1));
    return slot_no;
}

// IDA_EA: 0x8604898
// IDA_NAME: _ZNK9SkillSlot20check_skill_masteredEiii
// DEMANGLED: SkillSlot::check_skill_mastered(int,int,int)const
bool SkillSlot::check_skill_mastered(int skill_index, int level,
                                     int group) const {
    CUser* user = parent_;
    if (!user) {
        return false;
    }
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    const _Mastered_skill* const slots = get_skillslot_buf(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    if (!slots) {
        return false;
    }
    const int slot_no = get_skillslot_no(
        skill_index, group, static_cast<ENUM_SKILL_TREE_KIND>(skill_tree),
        true);
    if (slot_no < 0) {
        return false;
    }
    return slots[slot_no].skill_index == skill_index &&
           slots[slot_no].skill_level >= level;
}

// IDA_EA: 0x8604C02
// IDA_NAME: _ZNK9SkillSlot16check_skill_listEiSt6vectorISt4pairIiiESaIS2_EEi
// DEMANGLED: SkillSlot::check_skill_list(
//             int,std::vector<std::pair<int,int>>,int)const
int SkillSlot::check_skill_list(
    int charac_job,
    std::vector<std::pair<int, int> > required_skills,
    int level_offset) const {
    if (required_skills.empty()) {
        return 1;
    }

    for (std::vector<std::pair<int, int> >::const_iterator it =
             required_skills.begin();
         it != required_skills.end();
         ++it) {
        CSkill* const skill = G_CDataManager()->find_skill(charac_job,
                                                            it->first);
        if (!skill) {
            LogManager::logFormat(
                1,
                "skill_slot.cpp",
                "bool SkillSlot::check_skill_list(int, std::vector<std::pair<int, int>, std::allocator<std::pair<int, int> > >, int) const",
                0x42b,
                "G_CDataManager()->find_skill( %d, %d )",
                charac_job,
                it->first);
            return 0;
        }

        int required_level = it->second - level_offset;
        if (required_level <= 0) {
            required_level = 1;
        }
        if (!check_skill_mastered(it->first,
                                  required_level,
                                  skill->get_group())) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8604954
// IDA_NAME: _ZNK9SkillSlot20check_skill_masteredEiii20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::check_skill_mastered(int,int,int,ENUM_SKILL_TREE_KIND)const
bool SkillSlot::check_skill_mastered(int skill_index, int level, int group,
                                     ENUM_SKILL_TREE_KIND kind) const {
    if (!parent_) {
        return false;
    }
    const _Mastered_skill* const slots = get_skillslot_buf(kind);
    if (!slots) {
        return false;
    }
    const int slot_no = get_skillslot_no(skill_index, group, kind, true);
    if (slot_no < 0) {
        return false;
    }
    return slots[slot_no].skill_index == skill_index &&
           slots[slot_no].skill_level >= level;
}

// IDA_EA: 0x86049FC
// IDA_NAME: _ZNK9SkillSlot19get_skillslot_groupEi
// DEMANGLED: SkillSlot::get_skillslot_group(int)const
int SkillSlot::get_skillslot_group(int slot_index) const {
    int group = 6;
    if (slot_index > 5) {
        if (slot_index > 53) {
            if (slot_index > 101) {
                if (slot_index > 149) {
                    if (slot_index > 197) {
                        if (slot_index <= 203) {
                            group = -1;
                        }
                    } else {
                        group = 3;
                    }
                } else {
                    group = 2;
                }
            } else {
                group = 1;
            }
        } else {
            group = 0;
        }
    } else {
        group = -1;
    }
    if (group == 2 && slot_index > 137) {
        return 4;
    }
    return group;
}

// IDA_EA: 0x8604A86
// IDA_NAME: _ZNK9SkillSlot16get_skillslot_noEii20ENUM_SKILL_TREE_KINDb
// DEMANGLED: SkillSlot::get_skillslot_no(int,int,ENUM_SKILL_TREE_KIND,bool)const
int SkillSlot::get_skillslot_no(int skill_index, int group,
                                ENUM_SKILL_TREE_KIND kind,
                                bool quick_slot) const {
    if (!parent_) {
        return -1;
    }
    const _Mastered_skill* const slots = get_skillslot_buf(kind);
    if (!slots) {
        return -1;
    }
    if (quick_slot && checkComboSkillInsertQuickSlot(skill_index)) {
        for (int slot = 0; slot <= 5; ++slot) {
            if (slots[slot].skill_index == skill_index) {
                return slot;
            }
        }
        for (int slot = 198; slot <= 203; ++slot) {
            if (slots[slot].skill_index == skill_index) {
                return slot;
            }
        }
    }

    int begin_slot = 0;
    int end_slot = 0;
    if (group == 1) {
        begin_slot = 54;
        end_slot = 102;
    } else if (group > 1) {
        if (group == 2) {
            begin_slot = 102;
            end_slot = 150;
        } else if (group == 3) {
            begin_slot = 150;
            end_slot = 198;
        }
    } else if (!group) {
        begin_slot = 6;
        end_slot = 54;
    }
    for (int slot = begin_slot; slot < end_slot; ++slot) {
        if (slots[slot].skill_index == skill_index) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x8604D90
// IDA_NAME: _ZN9SkillSlot16clear_all_skillsE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::clear_all_skills(ENUM_SKILL_TREE_KIND)
int SkillSlot::clear_all_skills(ENUM_SKILL_TREE_KIND kind) {
    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    int* remain_sp = get_cur_remain_sp(kind);
    if (!slots || !remain_sp) {
        return 0;
    }
    parent_ = 0;
    *remain_sp = 0;
    std::memset(slots, 0, sizeof(_Mastered_skill) * kSkillSlotCount);
    return 1;
}

// IDA_EA: 0x8604E08
// IDA_NAME: _ZN9SkillSlot21clear_all_skills_bothEv
// DEMANGLED: SkillSlot::clear_all_skills_both(void)
int SkillSlot::clear_all_skills_both() {
    parent_ = 0;
    remain_sp_tree_0_ = 0;
    std::memset(skill_tree_0_, 0, sizeof(skill_tree_0_));
    remain_sp_tree_1_ = 0;
    std::memset(skill_tree_1_, 0, sizeof(skill_tree_1_));
    load_striker_skill_ = false;
    return 1;
}

// IDA_EA: 0x8608120
// IDA_NAME: _ZN9SkillSlot9InitSkillEb20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::InitSkill(bool,ENUM_SKILL_TREE_KIND)
int SkillSlot::InitSkill(bool send_skill_info,
                         ENUM_SKILL_TREE_KIND kind) {
    CUser* const user = parent_;
    if (!user) {
        return 0;
    }

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        const int target_tree =
            (kind != ENUM_SKILL_TREE_CURRENT && kind != ENUM_SKILL_TREE_0) ? 2 : 1;
        user->givePvPSkillTree(0, true, target_tree);
        const int pvp_skill_point =
            G_CDataManager()->get_pvp_skill_tree_parameter_script()
                ->getPvPSkillPoint(user->get_charac_job(),
                                   user->getCurCharFirstGrowType(),
                                   user->getCurCharSecondGrowType(),
                                   user->get_pvp_grade(),
                                   false);
        set_remain_sp_at_index(pvp_skill_point, kind);
        if (send_skill_info) {
            user->send_skill_info();
            cMyTrace trace("bool SkillSlot::InitSkill(bool, ENUM_SKILL_TREE_KIND)",
                           2887,
                           0);
            trace("SUCCESS_INIT_SKILL and SEND_SKILL_INFO, char(%s)",
                  user->getCurCharacName());
        }
        InitSkillHistoryLog(kind, 1);
        return 1;
    }

    addSkillOnCreateCharacter(get_mastered_skill_tree(kind),
                              user->get_charac_job());
    std::vector<std::pair<int, int> > give_skills;
    CCharacList* const character_list = G_CDataManager()->get_character_list();
    CCharacter* const character =
        &character_list->characters_[user->get_charac_job()];
    if (character->get_give_skill(user->getCurCharFirstGrowType(),
                                  user->getCurCharSecondGrowType(),
                                  give_skills,
                                  0) != 1) {
        return 0;
    }
    NormalizeAutomaticGiveSkillLevels(
        user, user->get_charac_job(), user->getCurCharFirstGrowType(),
        user->getCurCharSecondGrowType(), give_skills);
    for (std::vector<std::pair<int, int> >::const_iterator iter =
             give_skills.begin();
         iter != give_skills.end();
         ++iter) {
        growtype_skill(user->get_charac_job(), iter->first, iter->second, kind);
    }

    STExpertJobScript* const expert_job = G_CDataManager()->GetExpertJobScript(
        user->GetCurCharacExpertJobType());
    if (expert_job) {
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 expert_job->unknown_exp_pairs.begin();
             iter != expert_job->unknown_exp_pairs.end();
             ++iter) {
            growtype_skill(user->get_charac_job(),
                           iter->first,
                           user->GetCurExpertJobLevel(
                               user->GetCurCharacExpertJobExp()),
                           kind);
        }
    }

    WongWork::CSkillChanger skill_changer;
    const int target_tree =
        (kind != ENUM_SKILL_TREE_CURRENT && kind != ENUM_SKILL_TREE_0) ? 2 : 1;
    skill_changer.SkillInitialize(user, target_tree, false);
    if (send_skill_info) {
        user->send_skill_info();
        cMyTrace trace("bool SkillSlot::InitSkill(bool, ENUM_SKILL_TREE_KIND)",
                       2950,
                       0);
        trace("SUCCESS_INIT_SKILL and SEND_SKILL_INFO, char(%s)",
              user->getCurCharacName());
    }
    InitSkillHistoryLog(kind, 0);
    return 1;
}

// IDA_EA: 0x86085FA
// IDA_NAME: _ZN9SkillSlot19InitSkillHistoryLogE20ENUM_SKILL_TREE_KINDi
// DEMANGLED: SkillSlot::InitSkillHistoryLog(ENUM_SKILL_TREE_KIND,int)
CUserCharacInfo* SkillSlot::InitSkillHistoryLog(ENUM_SKILL_TREE_KIND kind,
                                                 int reason) {
    CUser* const user = parent_;
    if (!user) {
        return 0;
    }
    const int skill_tree =
        (kind != ENUM_SKILL_TREE_CURRENT && kind != ENUM_SKILL_TREE_0) ? 1 : 0;
    const int history_tree = skill_tree ? 3 : 2;
    CUser_history_log(user)->InitSkill(
        skill_tree,
        user->get_charac_level(),
        get_remain_sp_at_index(static_cast<ENUM_SKILL_TREE_KIND>(skill_tree)),
        get_remain_sfp_at_index(static_cast<ENUM_SKILL_TREE_KIND>(history_tree)),
        static_cast<eSkillInitReason>(reason));
    return user;
}

// IDA_EA: 0x8604E78
// IDA_NAME: _ZN9SkillSlot16clear_sfp_skillsEc20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::clear_sfp_skills(char,ENUM_SKILL_TREE_KIND)
int SkillSlot::clear_sfp_skills(char charac_job,
                                ENUM_SKILL_TREE_KIND kind) {
    for (int slot = 0; slot <= 203; ++slot) {
        if (kind == ENUM_SKILL_TREE_SFP_1) {
            CSkill* const skill = G_CDataManager()->find_skill(charac_job, slot);
            if (skill && skill->IsSpecialSkill()) {
                skill_tree_1_[slot].skill_index = 0;
                skill_tree_1_[slot].skill_level = 0;
            }
            continue;
        }

        if (kind == ENUM_SKILL_TREE_SFP_0) {
            CSkill* const skill = G_CDataManager()->find_skill(charac_job, slot);
            if (skill && skill->IsSpecialSkill()) {
                skill_tree_0_[slot].skill_index = 0;
                skill_tree_0_[slot].skill_level = 0;
            }
            continue;
        }

        LogManager::logFormat(
            1, "skill_slot.cpp",
            "bool SkillSlot::clear_sfp_skills(char, ENUM_SKILL_TREE_KIND)",
            0x49C, "SkillSlot::clear_sfp_skills error %d", kind);
    }
    return 1;
}

// IDA_EA: 0x8608A1E
// IDA_NAME: _ZN9SkillSlot20initialize_sfp_skillEv
// DEMANGLED: SkillSlot::initialize_sfp_skill(void)
int SkillSlot::initialize_sfp_skill() {
    CUser* const user = parent_;
    if (!user || user->get_charac_level() <= 49) {
        return 0;
    }

    const char charac_job = static_cast<char>(user->get_charac_job());
    clear_sfp_skills(charac_job, ENUM_SKILL_TREE_SFP_0);
    clear_sfp_skills(charac_job, ENUM_SKILL_TREE_SFP_1);
    const int remain_sfp = user->get_charac_level() - 49;
    set_remain_sfp_at_index(remain_sfp, ENUM_SKILL_TREE_SFP_0);
    return set_remain_sfp_at_index(remain_sfp, ENUM_SKILL_TREE_SFP_1);
}

// IDA_EA: 0x86047D8
// IDA_NAME: _ZN9SkillSlot12delete_skillEii20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::delete_skill(int,int,ENUM_SKILL_TREE_KIND)
int SkillSlot::delete_skill(int charac_job,
                            int skill_index,
                            ENUM_SKILL_TREE_KIND kind) {
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }

    if (!parent_) {
        return -1;
    }

    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    if (!slots) {
        return -1;
    }

    const int target_skill_index = skill->get_index();
    for (int slot = 0; slot <= 203; ++slot) {
        if (slots[slot].skill_index == target_skill_index) {
            slots[slot].skill_index = 0;
            slots[slot].skill_level = 0;
            return 0;
        }
    }
    return 0;
}

// IDA_EA: 0x860523E
// IDA_NAME: _ZN9SkillSlot22debugCommandResetSkillEv
// DEMANGLED: SkillSlot::debugCommandResetSkill(void)
int SkillSlot::debugCommandResetSkill() {
    CUser* const user = parent_;

    // The two 0x198-byte regions are the original tree-0/tree-1 skill
    // arrays at +0x46 and +0x1DE.  Both offsets and the 204 two-byte entries
    // come directly from the IDA body above.
    std::memset(skill_tree_0_, 0, sizeof(skill_tree_0_));
    std::memset(skill_tree_1_, 0, sizeof(skill_tree_1_));

    _Mastered_skill initial_skills[kSkillSlotCount];
    addSkillOnCreateCharacter(initial_skills, user->get_charac_job());

    for (int index = 0; index < kSkillSlotCount; ++index) {
        const _Mastered_skill& skill = initial_skills[index];
        if (skill.skill_index) {
            skill_tree_0_[index] = skill;
            skill_tree_1_[index] = skill;
        }
    }

    const int first_grow_type = user->getCurCharFirstGrowType();
    if (!first_grow_type) {
        return 0;
    }

    // IDA identifies set_grow_type as a void provider.  The original
    // decompiler's char return here is the residual call-register value;
    // the documented debug-command result is the nonzero grow-type branch.
    user->set_grow_type(static_cast<unsigned char>(first_grow_type),
        static_cast<unsigned char>(user->getCurCharSecondGrowType()), 0,
        static_cast<eChangeGrowTypeReason>(2));
    return 1;
}

// IDA_EA: 0x86066F2
// IDA_NAME: _ZNK9SkillSlot12makeCheckSumEi
// DEMANGLED: SkillSlot::makeCheckSum(int)const
int SkillSlot::makeCheckSum(int seed) const {
    CUser* const user = parent_;
    if (!user) {
        return 0;
    }

    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    const _Mastered_skill* const slots = get_skillslot_buf(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    if (!slots) {
        return -1;
    }

    int checksum = 0;
    for (int slot = 0; slot <= 203; ++slot) {
        const _Mastered_skill& entry = slots[slot];
        if (entry.skill_index) {
            checksum += getSkillChecksum(slot, entry.skill_index,
                                         entry.skill_level);
        }
    }
    return finishSkillChecksum(&checksum, seed);
}

// IDA_EA: 0x86067B8
// IDA_NAME: _ZNK9SkillSlot17GetSkillTreeIndexEv
// DEMANGLED: SkillSlot::GetSkillTreeIndex(void)const
int SkillSlot::GetSkillTreeIndex() const {
    CUser* user = parent_;
    if (user) {
        return user->GetCurCharacSkillTreeIndex();
    }
    return -1;
}

// IDA_EA: 0x86067DE
// IDA_NAME: _ZNK9SkillSlot17get_skillslot_bufE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::get_skillslot_buf(ENUM_SKILL_TREE_KIND)const
const _Mastered_skill* SkillSlot::get_skillslot_buf(
    ENUM_SKILL_TREE_KIND kind) const {
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        return skill_tree_0_;
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return skill_tree_1_;
    }
    return 0;
}

_Mastered_skill* SkillSlot::get_mastered_skill_tree(
    ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        return skill_tree_0_;
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return skill_tree_1_;
    }
    return 0;
}

// IDA_EA: 0x8606812
// IDA_NAME: _ZNK9SkillSlot17get_cur_remain_spE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::get_cur_remain_sp(ENUM_SKILL_TREE_KIND)const
int* SkillSlot::get_cur_remain_sp(ENUM_SKILL_TREE_KIND kind) const {
    if (!parent_) {
        return 0;
    }
    switch (kind) {
        case ENUM_SKILL_TREE_CURRENT:
        case ENUM_SKILL_TREE_0:
            return const_cast<int*>(&remain_sp_tree_0_);
        case ENUM_SKILL_TREE_1:
            return const_cast<int*>(&remain_sp_tree_1_);
        case ENUM_SKILL_TREE_SFP_0:
            return const_cast<int*>(&remain_sfp_tree_0_);
        case ENUM_SKILL_TREE_SFP_1:
            return const_cast<int*>(&remain_sfp_tree_1_);
    }
    return 0;
}

// IDA_EA: 0x8606874
// IDA_NAME: _ZNK9SkillSlot15get_skill_indexEi
// DEMANGLED: SkillSlot::get_skill_index(int)const
int SkillSlot::get_skill_index(int slot_index) const {
    CUser* user = parent_;
    if (!user) {
        return -1;
    }
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    if (skill_tree == -1 || !skill_tree) {
        return static_cast<unsigned char>(
            skill_tree_0_[slot_index].skill_index);
    }
    return static_cast<unsigned char>(skill_tree_1_[slot_index].skill_index);
}

// IDA_EA: 0x86068E6
// IDA_NAME: _ZNK9SkillSlot15get_skill_levelEi
// DEMANGLED: SkillSlot::get_skill_level(int)const
int SkillSlot::get_skill_level(int slot_index) const {
    CUser* user = parent_;
    if (!user) {
        return -1;
    }
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    if (skill_tree == -1 || !skill_tree) {
        return static_cast<signed char>(skill_tree_0_[slot_index].skill_level);
    }
    return static_cast<signed char>(skill_tree_1_[slot_index].skill_level);
}

// IDA_EA: 0x8606958
// IDA_NAME: _ZNK9SkillSlot16IsEmptySkillTreeE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::IsEmptySkillTree(ENUM_SKILL_TREE_KIND)const
int SkillSlot::IsEmptySkillTree(ENUM_SKILL_TREE_KIND kind) const {
    const _Mastered_skill* const slots = get_skillslot_buf(kind);
    if (!slots) {
        return 0;
    }
    for (int index = 0; index <= 203; ++index) {
        if (slots[index].skill_index) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8607DBA
// IDA_NAME: _ZNK9SkillSlot16get_skillslot_noEP15_Mastered_skilliib
// DEMANGLED: SkillSlot::get_skillslot_no(_Mastered_skill *,int,int,bool)const
int SkillSlot::get_skillslot_no(_Mastered_skill* skill_slot,
                                int skill_index,
                                int group,
                                bool quick_slot) const {
    if (!parent_) {
        return -1;
    }
    if (!skill_slot) {
        return -1;
    }
    if (quick_slot) {
        for (int slot_no = 0; slot_no <= 5; ++slot_no) {
            if (skill_slot[slot_no].skill_index == skill_index) {
                return slot_no;
            }
        }
    }

    int begin_slot = 0;
    int end_slot = 0;
    if (group == 1) {
        begin_slot = 54;
        end_slot = 102;
    } else if (group > 1) {
        if (group == 2) {
            begin_slot = 102;
            end_slot = 150;
        } else if (group == 3) {
            begin_slot = 150;
            end_slot = 198;
        }
    } else if (!group) {
        begin_slot = 6;
        end_slot = 54;
    }
    for (int slot_no = begin_slot; slot_no < end_slot; ++slot_no) {
        if (skill_slot[slot_no].skill_index == skill_index) {
            return slot_no;
        }
    }
    if (quick_slot) {
        for (int slot_no = 198; slot_no <= 203; ++slot_no) {
            if (skill_slot[slot_no].skill_index == skill_index) {
                return slot_no;
            }
        }
    }
    return -1;
}

// IDA_EA: 0x8607EFE
// IDA_NAME: _ZN9SkillSlot18get_temp_remain_spEv
// DEMANGLED: SkillSlot::get_temp_remain_sp(void)
int* SkillSlot::get_temp_remain_sp() {
    if (parent_) {
        return &temporary_remain_sp_;
    }
    return 0;
}

// IDA_EA: 0x8607F1C
// IDA_NAME: _ZN9SkillSlot19get_temp_remain_sfpEv
// DEMANGLED: SkillSlot::get_temp_remain_sfp(void)
int* SkillSlot::get_temp_remain_sfp() {
    if (parent_) {
        return &temporary_remain_sfp_;
    }
    return 0;
}

// IDA_EA: 0x8607F3A
// IDA_NAME: _ZN9SkillSlot19get_temp_skill_slotEv
// DEMANGLED: SkillSlot::get_temp_skill_slot(void)
_Mastered_skill* SkillSlot::get_temp_skill_slot() {
    if (parent_) {
        return temporary_tree_;
    }
    return 0;
}

// IDA_EA: 0x8607F58
// IDA_NAME: _ZN9SkillSlot11backupSkillE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::backupSkill(ENUM_SKILL_TREE_KIND)
int SkillSlot::backupSkill(ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0) {
        std::memcpy(temporary_tree_, skill_tree_0_, sizeof(temporary_tree_));
        temporary_remain_sp_ = remain_sp_tree_0_;
        temporary_remain_sfp_ = remain_sfp_tree_0_;
        return 1;
    }
    if (kind == ENUM_SKILL_TREE_1) {
        std::memcpy(temporary_tree_, skill_tree_1_, sizeof(temporary_tree_));
        temporary_remain_sp_ = remain_sp_tree_1_;
        temporary_remain_sfp_ = remain_sfp_tree_1_;
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8608014
// IDA_NAME: _ZNK9SkillSlot20check_skill_masteredEP15_Mastered_skilliii
// DEMANGLED: SkillSlot::check_skill_mastered(_Mastered_skill *,int,int,int)const
bool SkillSlot::check_skill_mastered(_Mastered_skill* skill_slot,
                                     int skill_index,
                                     int level, int group) const {
    const int slot_no =
        get_skillslot_no(skill_slot, skill_index, group, true);
    if (slot_no < 0) {
        return false;
    }
    return skill_slot[slot_no].skill_index == skill_index &&
           skill_slot[slot_no].skill_level >= level;
}

// IDA_EA: 0x86086AA
// IDA_NAME: _ZN9SkillSlot18IsLoadStrikerSkillEv
// DEMANGLED: SkillSlot::IsLoadStrikerSkill(void)
int SkillSlot::IsLoadStrikerSkill() {
    return load_striker_skill_;
}

// IDA_EA: 0x86086B6
// IDA_NAME: _ZN9SkillSlot19SetLoadStrikerSkillEb
// DEMANGLED: SkillSlot::SetLoadStrikerSkill(bool)
SkillSlot* SkillSlot::SetLoadStrikerSkill(bool loaded) {
    load_striker_skill_ = loaded;
    return this;
}

// IDA_EA: 0x86086CE
// IDA_NAME: _ZN9SkillSlot13SendSkillInfoEi20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::SendSkillInfo(int,ENUM_SKILL_TREE_KIND)
void SkillSlot::SendSkillInfo(int charac_idx, ENUM_SKILL_TREE_KIND kind) {
    CUser* user = parent_;
    if (!user) {
        return;
    }

    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_REQUEST_CHARAC_SKILL_INFO);
    response.put_byte(1);
    response.put_byte(charac_idx);

    const _Charac_info& character = user->characters_[charac_idx];
    response.put_byte(character.fields_.job_);
    response.put_byte(
        static_cast<char>(16 * character.fields_.grow_type_.growth_type) >> 4);

    int count_index = response.get_index();
    int skill_count = 0;
    response.put_byte(0);

    const _Mastered_skill* skill_slot = skill_tree_0_;
    if (kind != ENUM_SKILL_TREE_CURRENT && kind != ENUM_SKILL_TREE_0) {
        skill_slot = skill_tree_1_;
    }

    for (int slot = 0; slot < kSkillSlotCount; ++slot) {
        if (skill_slot[slot].skill_index) {
            response.put_byte(slot);
            response.put_byte(skill_slot[slot].skill_index);
            response.put_byte(skill_slot[slot].skill_level);
            ++skill_count;
        }
    }
    response.put_byte(count_index, skill_count);
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x86088E0
// IDA_NAME: _ZN9SkillSlot24checkskillSlotEmptyCountERi
// DEMANGLED: SkillSlot::checkskillSlotEmptyCount(int &)
int SkillSlot::checkskillSlotEmptyCount(int& kind) {
    CUser* user = parent_;
    if (!user) {
        return 0;
    }
    const int charac_level = user->get_charac_level();
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    const _Mastered_skill* const slots = get_skillslot_buf(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    if (!slots) {
        return 0;
    }

    int normal_empty = 0;
    for (int slot = 0; slot <= 5; ++slot) {
        if (!slots[slot].skill_index) {
            ++normal_empty;
        }
    }

    int result = normal_empty;
    if (charac_level > 15) {
        if (charac_level > 30) {
            kind = 3;
        } else {
            --normal_empty;
            kind = 2;
        }
    } else {
        normal_empty -= 3;
        kind = 1;
    }
    result = normal_empty;
    if (charac_level <= 45) {
        return result;
    }

    int combo_empty = 0;
    for (int slot = 198; slot <= 203; ++slot) {
        if (!slots[slot].skill_index) {
            ++combo_empty;
        }
    }

    int combo_result = 0;
    if (charac_level > 60) {
        combo_result = combo_empty - 2;
        kind = 5;
    } else {
        combo_result = combo_empty - 3;
        kind = 4;
    }
    if (combo_result > 0) {
        result += combo_result;
    }
    return result;
}

// IDA_EA: 0x8608B1E
// IDA_NAME: _ZN9SkillSlot14loadComboSkillEP15SIG_COMBO_SKILL
// DEMANGLED: SkillSlot::loadComboSkill(SIG_COMBO_SKILL *)
void SkillSlot::loadComboSkill(SIG_COMBO_SKILL* data) {
    if (!data) {
        return;
    }
    if (combo_skill_.loadData(data) != 1) {
        LogManager::logFormat(
            1, "skill_slot.cpp",
            "void SkillSlot::loadComboSkill(SIG_COMBO_SKILL*)",
            0xC75, "SkillSlot::loadComboSkill error");
    }
}

// IDA_EA: 0x8608B78
// IDA_NAME: _ZNK9SkillSlot21getComboSkillSaveDataEP15SIG_COMBO_SKILL
// DEMANGLED: SkillSlot::getComboSkillSaveData(SIG_COMBO_SKILL *)const
CUser* SkillSlot::getComboSkillSaveData(SIG_COMBO_SKILL* data) const {
    CUser* user = parent_;
    if (!user || !data) {
        return user;
    }

    data->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    data->charac_job = static_cast<uint32_t>(user->get_charac_job());
    if (combo_skill_.saveData(data) != 1) {
        LogManager::logFormat(
            1, "skill_slot.cpp",
            "void SkillSlot::getComboSkillSaveData(SIG_COMBO_SKILL*) const",
            0xC84, "SkillSlot::getComboSkillSaveData error");
    }
    return user;
}

// IDA_EA: 0x8608C12
// IDA_NAME: _ZNK9SkillSlot21getComboSkillSendInfoER11PacketGuard
// DEMANGLED: SkillSlot::getComboSkillSendInfo(PacketGuard &)const
int SkillSlot::getComboSkillSendInfo(PacketGuard& packet) const {
    return combo_skill_.getComboSkillSendList(packet);
}

// IDA_EA: 0x8608C30
// IDA_NAME: _ZN9SkillSlot17setComboSkillInfoE20ENUM_SKILL_TREE_KINDRSt3mapIsSt6vectorIiSaIiEESt4lessIsESaISt4pairIKsS4_EEE
// DEMANGLED: SkillSlot::setComboSkillInfo(ENUM_SKILL_TREE_KIND,std::map<short,std::vector<int,std::allocator<int>>,std::less<short>,std::allocator<std::pair<short const,std::vector<int,std::allocator<int>>>>> &)
int SkillSlot::setComboSkillInfo(ENUM_SKILL_TREE_KIND kind,
                                 ComboSkillMap& combo_map) {
    if (static_cast<unsigned int>(kind) < 2u) {
        return combo_skill_.setComboSkill(kind, combo_map);
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "void SkillSlot::setComboSkillInfo(ENUM_SKILL_TREE_KIND, ComboSkillMap&)",
        0xC92, "SkillSlot::setComboSkillInfo kind(%d) error", kind);
    return 0;
}

// IDA_EA: 0x8608C98
// IDA_NAME: _ZN9SkillSlot23checkMoveComboSkillSlotEhh
// DEMANGLED: SkillSlot::checkMoveComboSkillSlot(unsigned char,unsigned char)
bool SkillSlot::checkMoveComboSkillSlot(unsigned char src_slot,
                                        unsigned char dst_slot) {
    CUser* user = parent_;
    if (!user) {
        return false;
    }
    const bool blocked =
        user->get_charac_job() ==
            oldsrv::config::kCharacterJobDemonicSwordman &&
        (src_slot <= 5u || dst_slot <= 5u);
    return !blocked;
}

// IDA_EA: 0x8608CF6
// IDA_NAME: _ZN9SkillSlot15resetComboSkillE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::resetComboSkill(ENUM_SKILL_TREE_KIND)
int SkillSlot::resetComboSkill(ENUM_SKILL_TREE_KIND kind) {
    if (static_cast<unsigned int>(kind) < 2u) {
        return combo_skill_.resetTree(kind);
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "void SkillSlot::resetComboSkill(ENUM_SKILL_TREE_KIND)",
        0xCBA, "SkillSlot::resetComboSkill kind(%d) error", kind);
    return 0;
}

// IDA_EA: 0x8608D58
// IDA_NAME: _ZNK9SkillSlot30checkComboSkillInsertQuickSlotEi
// DEMANGLED: SkillSlot::checkComboSkillInsertQuickSlot(int)const
bool SkillSlot::checkComboSkillInsertQuickSlot(int skill_index) const {
    CUser* user = parent_;
    if (!user) {
        return false;
    }
    const bool blocked =
        user->get_charac_job() ==
            oldsrv::config::kCharacterJobDemonicSwordman &&
        !skill_index;
    return !blocked;
}

// IDA_EA: 0x8608DA4
// IDA_NAME: _ZN9SkillSlot32pullComboSkillExtensionQuickSlotE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::pullComboSkillExtensionQuickSlot(ENUM_SKILL_TREE_KIND)
char SkillSlot::pullComboSkillExtensionQuickSlot(
    ENUM_SKILL_TREE_KIND kind) {
    CUser* user = parent_;
    if (!user) {
        return 0;
    }

    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    if (!slots) {
        return 0;
    }

    for (int slot = 198; slot <= 203; ++slot) {
        _Mastered_skill& mastered = slots[slot];
        if (!mastered.skill_index) {
            continue;
        }

        if (moveComboSkillExtensionQuickSlot(mastered.skill_index,
                                             mastered.skill_level, kind)) {
            mastered.skill_index = 0;
            mastered.skill_level = 0;
        } else {
            LogManager::logFormat(
                1, "skill_slot.cpp",
                "void SkillSlot::pullComboSkillExtensionQuickSlot(ENUM_SKILL_TREE_KIND)",
                0xCD7,
                "pullComboSkillExtensionQuickSlot Error slot(%d) index(%d) level(%d)kind(%d) characNo(%d)",
                slot,
                static_cast<unsigned int>(mastered.skill_index),
                static_cast<int>(mastered.skill_level),
                static_cast<int>(kind),
                user->get_charac_no(-1));
        }
    }
    return 1;
}

// IDA_EA: 0x8608EE8
// IDA_NAME: _ZN9SkillSlot32moveComboSkillExtensionQuickSlotEii20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::moveComboSkillExtensionQuickSlot(int,int,ENUM_SKILL_TREE_KIND)
bool SkillSlot::moveComboSkillExtensionQuickSlot(
    int skill_index,
    int level,
    ENUM_SKILL_TREE_KIND kind) {
    CUser* user = parent_;
    if (!user) {
        return false;
    }

    _Mastered_skill* const slots = get_mastered_skill_tree(kind);
    if (!slots) {
        return false;
    }

    const int charac_job = user->get_charac_job();
    CSkill* skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return false;
    }

    const int group = skill->get_group();
    const int slot_no =
        get_skillslot_no(skill_index, group, kind, false);
    if (slot_no == -1) {
        const int inserted =
            insert_skill(skill_index, group, false, kind, 1);
        if (inserted <= 5 || inserted > 197) {
            LogManager::logFormat(
                1, "skill_slot.cpp",
                "bool SkillSlot::moveComboSkillExtensionQuickSlot(int, int, ENUM_SKILL_TREE_KIND)",
                0xCF9,
                "moveComboSkillExtensionQuickSlot Error slot(%d) index(%d) level(%d)kind(%d) characNo(%d)",
                inserted,
                static_cast<unsigned int>(slots[inserted].skill_index),
                static_cast<int>(slots[inserted].skill_level),
                static_cast<int>(kind),
                user->get_charac_no(-1));
        } else {
            slots[inserted].skill_level = static_cast<int8_t>(level);
        }
    } else if (slots[slot_no].skill_level < level) {
        slots[slot_no].skill_level = static_cast<int8_t>(level);
    }
    return true;
}

// IDA_EA: 0x86090A0
// IDA_NAME: _ZN9SkillSlot26eraseDuplicationComboSKillEv
// DEMANGLED: SkillSlot::eraseDuplicationComboSKill(void)
int SkillSlot::eraseDuplicationComboSKill() {
    eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_0);
    return eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_1);
}

// IDA_EA: 0x86090CE
// IDA_NAME: _ZN9SkillSlot30eraseDuplicationComboSKillTreeE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_KIND)
char SkillSlot::eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_KIND kind) {
    if (static_cast<unsigned int>(kind) >= 2u) {
        LogManager::logFormat(
            1, "skill_slot.cpp",
            "void SkillSlot::eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_KIND)",
            0xD0B,
            "SkillSlot::eraseDuplicationComboSKillTree kind(%d) error",
            kind);
        return 0;
    }

    for (int slot = 198; slot <= 203; ++slot) {
        const int skill_slot_index = getSkillSlotIndex(kind, slot);
        if (skill_slot_index) {
            combo_skill_.eraseDuplicationComboSKill(kind,
                                                    skill_slot_index);
        }
    }
    return 0;
}

// IDA_EA: 0x860917E
// IDA_NAME: _ZNK9SkillSlot17getSkillSlotIndexE20ENUM_SKILL_TREE_KINDi
// DEMANGLED: SkillSlot::getSkillSlotIndex(ENUM_SKILL_TREE_KIND,int)const
int SkillSlot::getSkillSlotIndex(ENUM_SKILL_TREE_KIND kind,
                                 int slot_index) const {
    if (kind <= ENUM_SKILL_TREE_1) {
        if (kind != ENUM_SKILL_TREE_CURRENT && kind != ENUM_SKILL_TREE_0) {
            return static_cast<unsigned char>(
                skill_tree_1_[slot_index].skill_index);
        }
        return static_cast<unsigned char>(
            skill_tree_0_[slot_index].skill_index);
    }
    LogManager::logFormat(
        1, "skill_slot.cpp",
        "int SkillSlot::getSkillSlotIndex(ENUM_SKILL_TREE_KIND, int) const",
        0xD1F, "SkillSlot::getSkillSlotIndex kind(%d) error", kind);
    return -1;
}

// IDA_EA: 0x86091FE
// IDA_NAME: _ZN9SkillSlot16verifyComboSkillEv
// DEMANGLED: SkillSlot::verifyComboSkill(void)
int SkillSlot::verifyComboSkill() {
    verifyComboSkillTree(ENUM_SKILL_TREE_0);
    return verifyComboSkillTree(ENUM_SKILL_TREE_1);
}

// IDA_EA: 0x860922C
// IDA_NAME: _ZN9SkillSlot20verifyComboSkillTreeE20ENUM_SKILL_TREE_KIND
// DEMANGLED: SkillSlot::verifyComboSkillTree(ENUM_SKILL_TREE_KIND)
char SkillSlot::verifyComboSkillTree(ENUM_SKILL_TREE_KIND kind) {
    if (static_cast<unsigned int>(kind) >= 2u) {
        LogManager::logFormat(
            1, "skill_slot.cpp",
            "void SkillSlot::verifyComboSkillTree(ENUM_SKILL_TREE_KIND)",
            0xD33, "SkillSlot::verifyComboSkillTree kind(%d) error",
            kind);
        return 0;
    }

    // The six job-9 initial-skill entries are the combo-container slots
    // themselves.  The official verifier removes only those container ids
    // from their command chains, preventing a combo from recursively invoking
    // another combo.  Walking the 204 learned-skill slots here would instead
    // erase every legitimate ASDFGH command before persistence.
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        return 0;
    }
    const std::vector<std::pair<int, int> >& combo_skill_indexes =
        data_manager->get_initial_character_skills(
            oldsrv::config::kCharacterJobDemonicSwordman);
    if (combo_skill_indexes.size() < 6u) {
        LogManager::logFormat(
            1, "skill_slot.cpp",
            "void SkillSlot::verifyComboSkillTree(ENUM_SKILL_TREE_KIND)",
            __LINE__,
            "[COMBO_SKILL][VERIFY_FAIL] reason=container_count count=%u expected=6",
            static_cast<unsigned int>(combo_skill_indexes.size()));
        return 0;
    }
    for (int index = 0; index <= 5; ++index) {
        const int combo_skill_index = combo_skill_indexes[index].first;
        if (combo_skill_index) {
            combo_skill_.eraseDuplicationComboSKill(kind,
                                                    combo_skill_index);
        }
    }
    return 0;
}

// IDA_EA: 0x8694E96
// IDA_NAME: _ZN9SkillSlot13set_remain_spEiiii
// DEMANGLED: SkillSlot::set_remain_sp(int,int,int,int)
SkillSlot* SkillSlot::set_remain_sp(int sp0, int sp1, int sfp0, int sfp1) {
    remain_sp_tree_0_ = sp0;
    remain_sp_tree_1_ = sp1;
    remain_sfp_tree_0_ = sfp0;
    remain_sfp_tree_1_ = sfp1;
    return this;
}

// IDA_EA: 0x8694EC0
// IDA_NAME: _ZNK9SkillSlot13get_remain_spEPiS0_S0_S0_
// DEMANGLED: SkillSlot::get_remain_sp(int *,int *,int *,int *)const
int* SkillSlot::get_remain_sp(int* sp0, int* sp1, int* sfp0,
                              int* sfp1) const {
    *sp0 = remain_sp_tree_0_;
    *sp1 = remain_sp_tree_1_;
    *sfp0 = remain_sfp_tree_0_;
    *sfp1 = remain_sfp_tree_1_;
    return sfp1;
}
