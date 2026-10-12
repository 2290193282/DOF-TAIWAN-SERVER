#include "old_data/PVF/creature/creature_script.h"
#include "common/config.h"
#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/data/datamanager/sync_script.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature_fillpoint_runtime.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


extern STScriptFileList g_creatureScriptFileList;
extern char g_creatureScriptBaseDirectory[256];

const char* ACTIVESTATUS_ATTACK_TAG[] = {
    "[slow]", "[freeze]", "[poison]", "[stun]", "[curse]", "[blind]",
    "[lightning]", "[stone]", "[sleep]", "[burn]", "[weapon break]",
    "[bleeding]", "[haste]", "[bless]", "[element]", "[confuse]",
    "[hold]", "[armor break]", "[none]", 0
};

const char* ACTIVE_STATUS_NO_BOX_BRACKET_TAG[] = {
    "slow", "freeze", "poison", "stun", "curse", "blind", "lightning",
    "stone", "sleep", "burn", "weapon break", "bleeding", "haste",
    "bless", "element", "confuse", "hold", "armor break", "none", 0
};

// IDA_EA: 0x88944FB
// IDA_NAME: _Z19getActiveStatusTypeRKSs
// DEMANGLED: getActiveStatusType(std::string const&)
int getActiveStatusType(const std::string& name) {
    for (int index = 0; index <= 17; ++index) {
        if (name == ACTIVESTATUS_ATTACK_TAG[index] ||
            name == ACTIVE_STATUS_NO_BOX_BRACKET_TAG[index]) {
            return index;
        }
    }
    return 18;
}

// IDA_EA: 0x83415E2
// IDA_NAME: _ZN10AttackInfoD2Ev
// DEMANGLED: AttackInfo::~AttackInfo()
AttackInfo::~AttackInfo() {}

// IDA_EA: 0x889037C
// IDA_NAME: _ZN10AttackInfoC2Ev
// DEMANGLED: AttackInfo::AttackInfo(void)
AttackInfo::AttackInfo()
    : damage(0.0f),
      damage_increase_rate(0.0f),
      absolute_damage(0),
      damage_bonus(0),
      attack_type(0),
      weapon_damage_apply(0),
      human_active_status_rate(1.0f),
      damage_reaction(1),
      push_aside(0),
      lift_up(0),
      critical_hit(0.0f),
      stuck(0.0f),
      stuck_probability_increase(0),
      knock_back(0),
      knock_back_rate(0.0f),
      weakness_rate(0.0f),
      weakness_time(0.0f),
      bounce(0),
      ignore_defense(0),
      hit_wav(),
      attack_direction(1),
      hit_type(5),
      show_blood(0),
      hit_stun_time(-1),
      hit_stun_rate(-1.0f),
      use_weapon_hit_delay_rate(0),
      hit_direction(0),
      ignore_super_armor(0),
      ignore_weight(0),
      force_hit_stun_time(0),
      hp_drain(0),
      mp_drain(0),
      unused_80(0),
      unused_84(100.0f),
      is_hit_number_increasing(1),
      module_type(3),
      ignore_diehard(0),
      hit_check_by_screen_pos(0),
      max_target_hit_number(-1),
      attack_myself(0),
      attack_friend(0),
      attack_enemy(1),
      elements(),
      active_status_type(),
      active_status_rate(),
      active_status_time(),
      unused_e0(),
      active_status_value(),
      active_status_data(),
      damage_rate(),
      catch_objects() {
    clear();
}

// IDA_EA: 0x88904B8
// IDA_NAME: _ZN10AttackInfo5clearEv
// DEMANGLED: AttackInfo::clear(void)
AttackInfo* AttackInfo::clear() {
    damage = 0.0f;
    damage_increase_rate = 0.0f;
    absolute_damage = 0;
    damage_bonus = 0;
    attack_type = 0;
    weapon_damage_apply = 0;
    human_active_status_rate = 1.0f;
    damage_reaction = 1;
    push_aside = 0;
    lift_up = 0;
    critical_hit = 0.0f;
    stuck = 0.0f;
    stuck_probability_increase = 0;
    knock_back = 0;
    knock_back_rate = 0.0f;
    weakness_rate = 0.0f;
    weakness_time = 0.0f;
    bounce = 0;
    ignore_defense = 0;
    hit_wav.clear();
    attack_direction = 1;
    hit_type = 5;
    show_blood = 0;
    hit_stun_time = -1;
    hit_stun_rate = -1.0f;
    use_weapon_hit_delay_rate = 0;
    hit_direction = 0;
    ignore_super_armor = 0;
    ignore_weight = 0;
    force_hit_stun_time = 0;
    hp_drain = 0;
    mp_drain = 0;
    unused_80 = 0;
    unused_84 = 100.0f;
    is_hit_number_increasing = 1;
    module_type = 3;
    ignore_diehard = 0;
    hit_check_by_screen_pos = 0;
    max_target_hit_number = -1;
    attack_myself = 0;
    attack_friend = 0;
    attack_enemy = 1;
    elements.clear();
    for (int index = 0; index <= 2; ++index) {
        active_status_type[index] = 18;
        active_status_rate[index] = 0.0f;
        active_status_time[index] = 0;
        unused_e0[index] = 0;
        active_status_value[index] = 0;
        active_status_data[index].clear();
        damage_rate[index] = 100.0f;
    }
    catch_objects.clear();
    return this;
}

// IDA_EA: 0x8890760
// IDA_NAME: _ZN10AttackInfo14isIntendDamageEv
// DEMANGLED: AttackInfo::isIntendDamage(void)
bool AttackInfo::isIntendDamage() {
    return weapon_damage_apply != 0 || damage > 0.0f || absolute_damage > 0;
}

// IDA_EA: 0x889079A
// IDA_NAME: _ZN10AttackInfo10setElementE12ENUM_ELEMENT
// DEMANGLED: AttackInfo::setElement(ENUM_ELEMENT)
void AttackInfo::setElement(ENUM_ELEMENT element) {
    const int element_value = static_cast<int>(element);
    if (element_value == 4) {
        elements.clear();
        return;
    }
    elements.erase(4);
    elements.insert(element_value);
}

// IDA_EA: 0x88907FE
// IDA_NAME: _ZN10AttackInfo14isValidElementE12ENUM_ELEMENT
// DEMANGLED: AttackInfo::isValidElement(ENUM_ELEMENT)
bool AttackInfo::isValidElement(ENUM_ELEMENT element) {
    const int element_value = static_cast<int>(element);
    if (element_value != 4) {
        return elements.find(element_value) != elements.end();
    }
    return elements.empty();
}

// IDA_EA: 0x88908B8
// IDA_NAME: _ZN10AttackInfo13setAttackTypeENS_15ENUM_ATTACKTYPEE
// DEMANGLED: AttackInfo::setAttackType(AttackInfo::ENUM_ATTACKTYPE)
AttackInfo* AttackInfo::setAttackType(ENUM_ATTACKTYPE value) {
    attack_type = value;
    return this;
}

// IDA_EA: 0x88908C6
// IDA_NAME: _ZN10AttackInfo13getAttackTypeERSs
// DEMANGLED: AttackInfo::getAttackType(std::string &)
int AttackInfo::getAttackType(std::string& name) {
    if (name == "attack type physical") {
        return 0;
    }
    if (name == "attack type magical") {
        return 1;
    }
    if (name == "attack type absolute") {
        return 2;
    }
    return 7;
}

// IDA_EA: 0x88D145A
// IDA_NAME: _Z22ImportAttackInfoScriptP10AttackInfoPKci
// DEMANGLED: ImportAttackInfoScript(AttackInfo *,char const*,int)
int ImportAttackInfoScript(AttackInfo* info,
                           const char* filename,
                           int module_type) {
    // The old i386 code reads direct scalar values through ScanInt(bool*),
    // including a null success pointer; ScanInt(int*) is status-only.
    if (loadRDARScriptFile(g_creatureScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    info->clear();
    std::string tag;
    std::string text;
    int active_status_index = 0;
    int active_status_module = -1;
    int element_module = -1;

    while (ScanType(tag, true) == 1) {
        int module_tag_index = -1;
        int parsed_module_type = -1;
        processModuleTagSet(module_tag_index,
                            parsed_module_type,
                            &tag,
                            module_type,
                            false);
        if (parsed_module_type != -1) {
            info->module_type = parsed_module_type;
        }

        if (tag == "[damage]") {
            info->damage = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[damage increase rate]") {
            info->damage_increase_rate = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[absolute damage]") {
            info->absolute_damage = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[damage bonus]") {
            info->damage_bonus = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[attack type]") {
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            if (text == "[physic]") {
                info->attack_type = AttackInfo::ATTACK_TYPE_PHYSIC;
            } else if (text == "[magic]") {
                info->attack_type = AttackInfo::ATTACK_TYPE_MAGIC;
            } else if (text == "[ignore defense]") {
                info->attack_type = AttackInfo::ATTACK_TYPE_IGNORE_DEFENSE;
            }
        } else if (tag == "[weapon damage apply]") {
            info->weapon_damage_apply = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[attack myself]") {
            info->attack_myself = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[attack friend]") {
            info->attack_friend = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[attack enemy]") {
            info->attack_enemy = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[elemental property]") {
            if (element_module != info->module_type) {
                info->elements.clear();
                element_module = info->module_type;
            }
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            info->setElement(static_cast<ENUM_ELEMENT>(
                getElement(text.c_str())));
        } else if (tag == "[active status]") {
            if (active_status_module != info->module_type) {
                for (int index = 0; index <= 2; ++index) {
                    info->active_status_value[index] = 0;
                    info->active_status_data[index].clear();
                }
                active_status_module = info->module_type;
            }
            if (active_status_index <= 2) {
                bool is_type = false;
                ScanTypeOrStr(is_type, text, true);
                info->active_status_type[active_status_index] =
                    getActiveStatusType(text);
                info->active_status_rate[active_status_index] =
                    ScanFloat(static_cast<bool*>(0));
                info->active_status_time[active_status_index] =
                    ScanInt(static_cast<bool*>(0));
                info->active_status_value[active_status_index] =
                    ScanInt(static_cast<bool*>(0));
                if (text == "[blind]" || text == "[slow]" ||
                    text == "[haste]") {
                    info->active_status_data[active_status_index].push_back(
                        ScanInt(static_cast<bool*>(0)));
                    info->active_status_data[active_status_index].push_back(
                        ScanInt(static_cast<bool*>(0)));
                } else if (text == "[stone]" || text == "[confuse]" ||
                           text == "[armor break]" ||
                           text == "[weapon break]" || text == "[poison]" ||
                           text == "[lightning]" || text == "[bleeding]") {
                    info->active_status_data[active_status_index].push_back(
                        ScanInt(static_cast<bool*>(0)));
                } else if (text == "[burn]" || text == "[bless]" ||
                           text == "[curse]") {
                    for (int index = 0; index <= 3; ++index) {
                        info->active_status_data[active_status_index].push_back(
                            ScanInt(static_cast<bool*>(0)));
                    }
                }
                ++active_status_index;
            }
        } else if (tag == "[damage reaction]") {
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            if (text == "[none]") {
                info->damage_reaction = 0;
            } else if (text == "[damage]") {
                info->damage_reaction = 1;
            } else if (text == "[down]") {
                info->damage_reaction = 2;
            } else if (text == "[damage except human]") {
                info->damage_reaction = 3;
            }
        } else if (tag == "[push aside]") {
            info->push_aside = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[lift up]") {
            info->lift_up = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[critical hit]") {
            info->critical_hit = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[stuck]") {
            info->stuck = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[stuck prob increase]") {
            info->stuck_probability_increase =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[knuck back]") {
            info->knock_back = ScanInt(static_cast<bool*>(0));
            if (info->knock_back == 3) {
                info->knock_back_rate = ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[weakness]") {
            info->weakness_rate = ScanFloat(static_cast<bool*>(0));
            info->weakness_time = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[bounce]") {
            info->bounce = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ignore defense]") {
            info->ignore_defense = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[human damage rate]") {
            info->damage_rate[0] = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[fort damage rate]") {
            info->damage_rate[1] = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[monster damage rate]") {
            info->damage_rate[2] = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[HP drain]") {
            info->hp_drain = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[MP drain]") {
            info->mp_drain = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[hit wav]") {
            ScanStr(&info->hit_wav);
        } else if (tag == "[attack direction]") {
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            if (text == "[hit lift up]") {
                info->attack_direction = 0;
            } else if (text == "[hit horizon]") {
                info->attack_direction = 1;
            } else if (text == "[hit down]") {
                info->attack_direction = 2;
            }
        } else if (tag == "[hit info]") {
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            if (text == "[blow]") {
                info->hit_type = 0;
            } else if (text == "[cut]") {
                info->hit_type = 1;
            } else if (text == "[burn]") {
                info->hit_type = 2;
            } else if (text == "[mana burn]") {
                info->hit_type = 4;
            } else if (text == "[etc]") {
                info->hit_type = 3;
            }
            ScanTypeOrStr(is_type, text, true);
            if (text == "[no blood]") {
                info->show_blood = 0;
            } else if (text == "[blood]") {
                info->show_blood = 1;
            }
            info->hit_stun_time = ScanInt(static_cast<bool*>(0));
            info->hit_stun_rate = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[is use weapon hit delay rate]") {
            info->use_weapon_hit_delay_rate =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[force hit stun time]") {
            info->force_hit_stun_time = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[hit direction]") {
            bool is_type = false;
            ScanTypeOrStr(is_type, text, true);
            if (text == "[auto]") {
                info->hit_direction = 0;
            } else if (text == "[front]") {
                info->hit_direction = 1;
            } else if (text == "[back]") {
                info->hit_direction = 2;
            } else if (text == "[outer]") {
                info->hit_direction = 3;
            } else if (text == "[inner]") {
                info->hit_direction = 4;
            }
        } else if (tag == "[ignore super armor]") {
            info->ignore_super_armor = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ignore weight]") {
            info->ignore_weight = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[human active status rate]") {
            info->human_active_status_rate =
                ScanFloat(static_cast<bool*>(0)) / 100.0f;
        } else if (tag == "[is hit number increasing]") {
            info->is_hit_number_increasing =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[catch object]") {
            info->catch_objects.clear();
            bool success = false;
            while (true) {
                CatchObjectInfo value;
                value.object_type = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.object_count = ScanInt(static_cast<bool*>(0));
                value.probability = ScanFloat(static_cast<bool*>(0));
                info->catch_objects.push_back(value);
            }
        } else if (tag == "[max target hit number]") {
            info->max_target_hit_number = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ignore diehard]") {
            info->ignore_diehard = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[hit check by screen pos]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            info->hit_check_by_screen_pos = value != 0;
        }
    }

    unloadRDARScriptFile();
    return 1;
}

// IDA_EA: 0x83467C8
// IDA_NAME: _ZN27CreatureAdditionalAnimationD2Ev
// DEMANGLED: CreatureAdditionalAnimation::~CreatureAdditionalAnimation()
CreatureAdditionalAnimation::~CreatureAdditionalAnimation() {}

// IDA_EA: 0x8A3DCC2
// IDA_NAME: _ZN27CreatureAdditionalAnimationC2EiiPKc
// DEMANGLED: CreatureAdditionalAnimation::CreatureAdditionalAnimation(int,int,char const*)
CreatureAdditionalAnimation::CreatureAdditionalAnimation(int index_value,
                                                         int category_value,
                                                         const char* name_value)
    : index(index_value),
      category(category_value),
      name(name_value) {}

// IDA_EA: 0x8A4F458
// IDA_NAME: _ZN27CreatureAdditionalAnimationC2ERKS_
// DEMANGLED: CreatureAdditionalAnimation::CreatureAdditionalAnimation(CreatureAdditionalAnimation const&)
CreatureAdditionalAnimation::CreatureAdditionalAnimation(
    const CreatureAdditionalAnimation& rhs)
    : index(rhs.index),
      category(rhs.category),
      name(rhs.name) {}

// IDA_EA: 0x8A4F51C
// IDA_NAME: _ZN27CreatureAdditionalAnimationaSERKS_
// DEMANGLED: CreatureAdditionalAnimation::operator=(CreatureAdditionalAnimation const&)
CreatureAdditionalAnimation& CreatureAdditionalAnimation::operator=(
    const CreatureAdditionalAnimation& rhs) {
    index = rhs.index;
    category = rhs.category;
    name = rhs.name;
    return *this;
}

// IDA_EA: 0x8A4FA8A
// IDA_NAME: _ZN21st_crature_id_item_idD2Ev
// DEMANGLED: st_crature_id_item_id::~st_crature_id_item_id()
st_crature_id_item_id::~st_crature_id_item_id() {}

// IDA_EA: 0x83458A8
// IDA_NAME: _ZN23CreatureScriptSkillInfoD2Ev
// DEMANGLED: CreatureScriptSkillInfo::~CreatureScriptSkillInfo()
CreatureScriptSkillInfo::~CreatureScriptSkillInfo() {}

// IDA_EA: 0x8A3DD26
// IDA_NAME: _ZN23CreatureScriptSkillInfoC2Ev
// DEMANGLED: CreatureScriptSkillInfo::CreatureScriptSkillInfo(void)
CreatureScriptSkillInfo::CreatureScriptSkillInfo()
    : skill_name(),
      skill_explain(),
      skill_recovery_time(0),
      learn_skill_level(0),
      skill_mp(0),
      skill_motion(),
      skill_under_effect(),
      skill_over_effect(),
      skill_info(),
      attack_info(),
      preload_passive_indices() {
    clear();
}

// IDA_EA: 0x8A3DE50
// IDA_NAME: _ZN23CreatureScriptSkillInfo5clearEv
// DEMANGLED: CreatureScriptSkillInfo::clear(void)
void CreatureScriptSkillInfo::clear() {
    skill_name.clear();
    skill_explain.clear();
    skill_recovery_time = 0;
    learn_skill_level = 1;
    skill_mp = -1;
    skill_motion.clear();
    skill_under_effect.clear();
    skill_over_effect.clear();
    skill_info.clear();
    attack_info.clear();
    preload_passive_indices.clear();
}

// IDA_EA: 0x8A40BBE
// IDA_NAME: _ZN23CreatureScriptSkillInfoC2ERKS_
// DEMANGLED: CreatureScriptSkillInfo::CreatureScriptSkillInfo(CreatureScriptSkillInfo const&)
CreatureScriptSkillInfo::CreatureScriptSkillInfo(
    const CreatureScriptSkillInfo& rhs)
    : skill_name(rhs.skill_name),
      skill_explain(rhs.skill_explain),
      skill_recovery_time(rhs.skill_recovery_time),
      learn_skill_level(rhs.learn_skill_level),
      skill_mp(rhs.skill_mp),
      skill_motion(rhs.skill_motion),
      skill_under_effect(rhs.skill_under_effect),
      skill_over_effect(rhs.skill_over_effect),
      skill_info(rhs.skill_info),
      attack_info(rhs.attack_info),
      preload_passive_indices(rhs.preload_passive_indices) {}

// IDA_EA: 0x833DF50
// IDA_NAME: _ZN25selectiveGrowCreatureInfoC2Ev
// DEMANGLED: selectiveGrowCreatureInfo::selectiveGrowCreatureInfo(void)
selectiveGrowCreatureInfo::selectiveGrowCreatureInfo() { clear(); }

// IDA_EA: 0x833DF64
// IDA_NAME: _ZN25selectiveGrowCreatureInfo5clearEv
// DEMANGLED: selectiveGrowCreatureInfo::clear(void)
selectiveGrowCreatureInfo* selectiveGrowCreatureInfo::clear() {
    using_exp_item = false;
    grow_level = -1;
    grow_a_type_item_index = -1;
    grow_b_type_item_index = -1;
    grow_a_type_creature_id = -1;
    grow_b_type_creature_id = -1;
    item_count_for_levelup = 1;
    return this;
}

// IDA_EA: 0x833DFAC
// IDA_NAME: _ZN14CreatureScript5clearEv
// DEMANGLED: CreatureScript::clear(void)
int CreatureScript::clear() {
    evolution_creature_ids.clear();
    index = 0;
    name.clear();
    width = 0;
    height = 0;
    floating_height = 0;
    draw_layer = 1;
    character_span = -50;
    skill_recovery_time = 0;
    over_skill_recovery_time = 0;
    direction_correct = true;
    for (int speed_index = 0; speed_index <= 4; ++speed_index) {
        move_speed[speed_index] = 0;
    }
    gravity = 1500;
    start_level = 1;
    permission_level = 1;
    max_level = 1;
    parent_max_level = 1;
    for (int slot_index = 0; slot_index <= 2; ++slot_index) {
        artifact_slot[slot_index] = 0;
    }
    learn_overskill_level = 1;
    skill_mp = 0;
    over_skill_mp = 0;
    skill_type = 0;
    over_skill_type = 0;
    basic_motion.clear();
    walk_motion.clear();
    run_motion.clear();
    fly_motion.clear();
    get_exp_motion.clear();
    get_exp_over_motion.clear();
    get_exp_under_motion.clear();
    skill_motion.clear();
    over_skill_motion.clear();
    response_motion.clear();
    special_motion.clear();
    equip_motion.clear();
    equip_over_motion.clear();
    equip_under_motion.clear();
    basic_over_effect.clear();
    walk_over_effect.clear();
    run_over_effect.clear();
    unused_motion_over_effect.clear();
    skill_over_effect.clear();
    over_skill_over_effect.clear();
    response_over_effect.clear();
    special_over_effect.clear();
    basic_under_effect.clear();
    walk_under_effect.clear();
    run_under_effect.clear();
    unused_motion_under_effect.clear();
    skill_under_effect.clear();
    over_skill_under_effect.clear();
    response_under_effect.clear();
    special_under_effect.clear();
    additional_animations.clear();
    default_particle.clear();
    response_sound_tag.clear();
    wing_sound_tag.clear();
    walk_sound_tag.clear();
    run_sound_tag.clear();
    revision_x = 0;
    revision_y = 0;
    etc_motions.clear();
    attack_infos.clear();
    skill_strings.clear();
    skill_level_values.clear();
    over_skill_level_values.clear();
    piercing.clear();
    int_data.clear();
    string_data.clear();
    script_path.clear();
    evolution_quest = 0;
    evolution_creature_id = 0;
    evolution_level = 0;
    selective_grow_info.clear();
    random_motions.clear();
    default_random_motion_rate = 0;
    increase_random_motion_rate = 0;
    skill_balloon_position.enabled = 0;
    skill_balloon_position.x = 0;
    skill_balloon_position.y = 0;
    over_skill_balloon_position.enabled = 0;
    over_skill_balloon_position.x = 0;
    over_skill_balloon_position.y = 0;
    upgrade_probability_increase = 0;
    special_feature.type = CREATURE_SPECIAL_FEATURE_NONE;
    special_feature.town_upgrade_feedback.interaction_type.clear();
    special_feature.town_upgrade_feedback.base_animation.clear();
    special_feature.town_upgrade_feedback.working_animation.clear();
    special_feature.town_upgrade_feedback.great_success_animation.clear();
    special_feature.town_upgrade_feedback.success_animation.clear();
    special_feature.town_upgrade_feedback.fail_animation.clear();
    special_feature.town_upgrade_feedback.big_fail_animation.clear();
    special_feature.motion_change.sub_directory.clear();
    special_feature.motion_change.height = 0;
    special_feature.spin_around.duration_ms = 0;
    special_feature.spin_around.spin_type = 0;
    special_feature.spin_around.apply_target.clear();
    evolution_item_id = 0;
    item_evolution_creature_ids.clear();
    skill_info.clear();
    over_skill_info.clear();
    using_random_skill = false;
    skill_infos.clear();
    over_skill_infos.clear();
    event_evolute_creatures.clear();
    return 0;
}

// IDA_EA: 0x833E5AC
// IDA_NAME: _ZN14CreatureScriptC2Ev
// DEMANGLED: CreatureScript::CreatureScript(void)
CreatureScript::CreatureScript() { clear(); }

// IDA_EA: 0x8340CA4
// IDA_NAME: _ZN14CreatureScriptD2Ev
// DEMANGLED: CreatureScript::~CreatureScript()
CreatureScript::~CreatureScript() {}

static bool LastConsumedTypeIs(const char* expected) {
    return expected != 0 && g_dnfLex.getLastToken() == DNF_TOK_TYPE &&
           std::strcmp(g_dnfLex.getType(), expected) == 0;
}

static bool ScanClosedStringList(std::vector<std::string>* destination,
                                 const char* closing_tag,
                                 const char* filename) {
    if (!destination || !closing_tag) return false;
    std::vector<std::string> parsed;
    std::string value;
    while (ScanStr(&value) == 1) {
        parsed.push_back(value);
    }
    if (!LastConsumedTypeIs(closing_tag)) {
        std::fprintf(stderr,
                     "[creature-parser] expected %s after string list: %s\n",
                     closing_tag,
                     filename ? filename : "(null)");
        return false;
    }
    destination->swap(parsed);
    return true;
}

static bool ScanClosedIntList(std::vector<int>* destination,
                              const char* closing_tag,
                              const char* filename) {
    if (!destination || !closing_tag) return false;
    std::vector<int> parsed;
    while (true) {
        bool success = false;
        const int value = ScanInt(&success);
        if (!success) break;
        parsed.push_back(value);
    }
    if (!LastConsumedTypeIs(closing_tag)) {
        std::fprintf(stderr,
                     "[creature-parser] expected %s after integer list: %s\n",
                     closing_tag,
                     filename ? filename : "(null)");
        return false;
    }
    destination->swap(parsed);
    return true;
}

class CreatureScriptCacheLoadGuard {
public:
    CreatureScriptCacheLoadGuard() : committed_(false) {
        CreatureFillPoint::BeginScriptCacheLoad();
    }

    ~CreatureScriptCacheLoadGuard() {
        if (!committed_) CreatureFillPoint::AbortScriptCacheLoad();
    }

    void Commit() {
        CreatureFillPoint::CommitScriptCacheLoad();
        committed_ = true;
    }

private:
    CreatureScriptCacheLoadGuard(const CreatureScriptCacheLoadGuard&);
    CreatureScriptCacheLoadGuard& operator=(
        const CreatureScriptCacheLoadGuard&);

    bool committed_;
};

// IDA_EA: 0x8A35E47
// IDA_NAME: _Z20importCreatureScriptP14CreatureScriptPKc
// DEMANGLED: importCreatureScript(CreatureScript *,char const*)
int importCreatureScript(CreatureScript* script, const char* filename) {
    // IDA 0x8A35F48: [character span] calls _Z7ScanIntPb with a null bool*.
    if (loadRDARScriptFile(g_creatureScriptBaseDirectory, filename) !=
        1) {
        std::fprintf(stderr,
                     "[creature-parser] RDAR open fail: %s%s\n",
                     g_creatureScriptBaseDirectory,
                     filename ? filename : "(null)");
        LogManager::logFormat(0,
                              "../RDARScriptObject.cpp",
                              "importCreatureScript",
                              0x2517,
                              "creature RDAR open fail : %s%s",
                              g_creatureScriptBaseDirectory,
                              filename ? filename : "(null)");
        return 0;
    }

    script->clear();
    getDirectoryPath(script->script_path,
                     "%s%s",
                     g_creatureScriptBaseDirectory,
                     filename);

    std::string type;
    std::string text;
    int value = 0;
    bool success = false;

    while (ScanType(type, true) == 1) {
        if (type == "[name]") {
            ScanStr(&script->name);
        } else if (type == "[character span]") {
            script->character_span = ScanInt(static_cast<bool*>(0));
        } else if (type == "[width]") {
            script->width = ScanInt(static_cast<bool*>(0));
            script->height = ScanInt(static_cast<bool*>(0));
        } else if (type == "[floating height]") {
            script->floating_height = ScanInt(static_cast<bool*>(0));
        } else if (type == "[layer]") {
            ScanStr(&text);
            script->draw_layer = getDrawLayer(text.c_str());
        } else if (type == "[skill recovery time]") {
            script->skill_recovery_time = ScanInt(static_cast<bool*>(0));
        } else if (type == "[over skill recovery time]") {
            script->over_skill_recovery_time = ScanInt(static_cast<bool*>(0));
        } else if (type == "[direction correct]") {
            script->direction_correct =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (type == "[move speed]") {
            for (unsigned int index = 0; index <= 4; ++index) {
                script->move_speed[index] = ScanInt(static_cast<bool*>(0));
            }
        } else if (type == "[gravity]") {
            script->gravity = ScanInt(static_cast<bool*>(0));
        } else if (type == "[start level]") {
            script->start_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[permission level]") {
            script->permission_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[max level]") {
            script->max_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[parent max level]") {
            script->parent_max_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[artifact slot]") {
            bool slots[3] = {false, false, false};
            while (ScanStr(&text) == 1) {
                if (text == "red") {
                    slots[0] = true;
                } else if (text == "blue") {
                    slots[1] = true;
                } else if (text == "green") {
                    slots[2] = true;
                } else {
                    std::fprintf(stderr,
                                 "[creature-parser] unknown artifact slot "
                                 "%s: %s\n",
                                 text.c_str(),
                                 filename);
                    return 0;
                }
            }
            if (!LastConsumedTypeIs("[/artifact slot]")) return 0;
            script->artifact_slot[0] = slots[0] ? 1 : 0;
            script->artifact_slot[1] = slots[1] ? 1 : 0;
            script->artifact_slot[2] = slots[2] ? 1 : 0;
        } else if (type == "[learn overskill level]") {
            script->learn_overskill_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[skill MP]") {
            script->skill_mp = ScanInt(static_cast<bool*>(0));
        } else if (type == "[over skill MP]") {
            script->over_skill_mp = ScanInt(static_cast<bool*>(0));
        } else if (type == "[skill type]") {
            if (ScanStr(&text) != 1) return 0;
            if (text == "[active]") {
                script->skill_type = 0;
            } else if (text == "[passive]") {
                script->skill_type = 1;
            } else {
                return 0;
            }
        } else if (type == "[over skill type]") {
            if (ScanStr(&text) != 1) return 0;
            if (text == "[active]") {
                script->over_skill_type = 0;
            } else if (text == "[passive]") {
                script->over_skill_type = 1;
            } else {
                return 0;
            }
        } else if (type == "[basic motion]") {
            ScanStr(&script->basic_motion);
        } else if (type == "[walk motion]") {
            ScanStr(&script->walk_motion);
        } else if (type == "[run motion]") {
            ScanStr(&script->run_motion);
        } else if (type == "[fly motion]") {
            ScanStr(&script->fly_motion);
        } else if (type == "[get exp motion]") {
            ScanStr(&script->get_exp_motion);
        } else if (type == "[get exp over motion]") {
            ScanStr(&script->get_exp_over_motion);
        } else if (type == "[get exp under motion]") {
            ScanStr(&script->get_exp_under_motion);
        } else if (type == "[skill motion]") {
            ScanStr(&script->skill_motion);
        } else if (type == "[over skill motion]") {
            ScanStr(&script->over_skill_motion);
        } else if (type == "[response motion]") {
            ScanStr(&script->response_motion);
        } else if (type == "[special motion]") {
            ScanStr(&script->special_motion);
        } else if (type == "[equip motion]") {
            ScanStr(&script->equip_motion);
        } else if (type == "[equip over motion]") {
            ScanStr(&script->equip_over_motion);
        } else if (type == "[equip under motion]") {
            ScanStr(&script->equip_under_motion);
        } else if (type == "[basic under effect]") {
            ScanStr(&script->basic_under_effect);
        } else if (type == "[walk under effect]") {
            ScanStr(&script->walk_under_effect);
        } else if (type == "[run under effect]") {
            ScanStr(&script->run_under_effect);
        } else if (type == "[skill under effect]") {
            ScanStr(&script->skill_under_effect);
        } else if (type == "[over skill under effect]") {
            ScanStr(&script->over_skill_under_effect);
        } else if (type == "[response under effect]") {
            ScanStr(&script->response_under_effect);
        } else if (type == "[special under effect]") {
            ScanStr(&script->special_under_effect);
        } else if (type == "[basic over effect]") {
            ScanStr(&script->basic_over_effect);
        } else if (type == "[walk over effect]") {
            ScanStr(&script->walk_over_effect);
        } else if (type == "[run over effect]") {
            ScanStr(&script->run_over_effect);
        } else if (type == "[skill over effect]") {
            ScanStr(&script->skill_over_effect);
        } else if (type == "[over skill over effect]") {
            ScanStr(&script->over_skill_over_effect);
        } else if (type == "[response over effect]") {
            ScanStr(&script->response_over_effect);
        } else if (type == "[special over effect]") {
            ScanStr(&script->special_over_effect);
        } else if (type == "[additional animation]") {
            // CLIENT_EA: ImportCreatureScript @0x158ACE0, pseudocode
            // 0x158C590..0x158C66D.  The first token selects a sparse motion
            // category, followed by the animation index and filename.  The
            // old reconstruction left animation_index at -1 and discarded
            // every official entry.
            std::string motion_category;
            std::string animation_name;
            if (ScanStr(&motion_category) != 1) return 0;

            int animation_category = -1;
            if (motion_category == "[basic]") {
                animation_category = 0;
            } else if (motion_category == "[walk]") {
                animation_category = 1;
            } else if (motion_category == "[run]") {
                animation_category = 2;
            } else if (motion_category == "[skill]") {
                animation_category = 4;
            } else if (motion_category == "[over skill]") {
                animation_category = 5;
            } else if (motion_category == "[response]") {
                animation_category = 6;
            } else if (motion_category == "[special]") {
                animation_category = 8;
            } else {
                std::fprintf(stderr,
                             "[creature-parser] unknown additional animation "
                             "category %s: %s\n",
                             motion_category.c_str(),
                             filename);
                return 0;
            }

            const int animation_index = ScanInt(&success);
            if (!success || ScanStr(&animation_name) != 1) return 0;
            CreatureAdditionalAnimation animation(animation_index,
                                                  animation_category,
                                                  animation_name.c_str());
            script->additional_animations.push_back(animation);
        } else if (type == "[default particle]") {
            ScanStr(&script->default_particle);
        } else if (type == "[response sound tag]") {
            ScanStr(&script->response_sound_tag);
        } else if (type == "[wing sound tag]") {
            ScanStr(&script->wing_sound_tag);
        } else if (type == "[walk sound tag]") {
            ScanStr(&script->walk_sound_tag);
        } else if (type == "[run sound tag]") {
            ScanStr(&script->run_sound_tag);
        } else if (type == "[revision x]") {
            script->revision_x = ScanInt(static_cast<bool*>(0));
        } else if (type == "[revision y]") {
            script->revision_y = ScanInt(static_cast<bool*>(0));
        } else if (type == "[default random motion rate]") {
            script->default_random_motion_rate =
                ScanInt(static_cast<bool*>(0));
        } else if (type == "[increase random motion rate]") {
            script->increase_random_motion_rate =
                ScanInt(static_cast<bool*>(0));
        } else if (type == "[upgrade prob increase]") {
            script->upgrade_probability_increase =
                ScanInt(static_cast<bool*>(0));
        } else if (type == "[creature special feature]") {
            std::string feature_type;
            if (ScanType(feature_type, true) != 1) {
                std::fprintf(stderr,
                             "[creature-parser] missing special feature type: %s\n",
                             filename);
                return 0;
            }

            std::string key;
            std::string closing_type;
            if (feature_type == "[town feedback stay motion]") {
                CreatureTownUpgradeFeedbackMotion& feedback =
                    script->special_feature.town_upgrade_feedback;
                script->special_feature.type =
                    CREATURE_SPECIAL_FEATURE_TOWN_UPGRADE_FEEDBACK;

                if (ScanStr(&key) != 1 || key != "type" ||
                    ScanStr(&feedback.interaction_type) != 1 ||
                    ScanStr(&key) != 1 || key != "[motion]") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid town feedback header: %s\n",
                                 filename);
                    return 0;
                }

                const char* const motion_names[] = {
                    "base", "working", "GreatSuccess",
                    "Success", "Fail", "BigFail"
                };
                std::string* const motion_values[] = {
                    &feedback.base_animation,
                    &feedback.working_animation,
                    &feedback.great_success_animation,
                    &feedback.success_animation,
                    &feedback.fail_animation,
                    &feedback.big_fail_animation
                };
                for (unsigned int index = 0; index < 6; ++index) {
                    if (ScanStr(&key) != 1 || key != motion_names[index] ||
                        ScanStr(motion_values[index]) != 1) {
                        std::fprintf(
                            stderr,
                            "[creature-parser] invalid town feedback motion: %s\n",
                            filename);
                        return 0;
                    }
                }
                if (ScanStr(&key) != 1 || key != "[/motion]" ||
                    ScanType(closing_type, true) != 1 ||
                    closing_type != "[/town feedback stay motion]") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid town feedback closure: %s\n",
                                 filename);
                    return 0;
                }
            } else if (feature_type == "[change motion]") {
                CreatureMotionChangeFeature& motion =
                    script->special_feature.motion_change;
                script->special_feature.type =
                    CREATURE_SPECIAL_FEATURE_CHANGE_MOTION;
                if (ScanStr(&key) != 1 || key != "sub dir" ||
                    ScanStr(&motion.sub_directory) != 1 ||
                    ScanStr(&key) != 1 || key != "height") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid motion change header: %s\n",
                                 filename);
                    return 0;
                }
                motion.height = ScanInt(&success);
                if (!success || ScanType(closing_type, true) != 1 ||
                    closing_type != "[/change motion]") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid motion change value: %s\n",
                                 filename);
                    return 0;
                }
            } else if (feature_type == "[spin around]") {
                CreatureSpinAroundFeature& spin =
                    script->special_feature.spin_around;
                script->special_feature.type =
                    CREATURE_SPECIAL_FEATURE_SPIN_AROUND;
                if (ScanStr(&key) != 1 || key != "spin time") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid spin duration key: %s\n",
                                 filename);
                    return 0;
                }
                spin.duration_ms = ScanInt(&success);
                if (!success || ScanStr(&key) != 1 || key != "SpinType") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid spin duration: %s\n",
                                 filename);
                    return 0;
                }
                spin.spin_type = ScanInt(&success);
                if (!success || ScanStr(&key) != 1 || key != "ApplyTo" ||
                    ScanStr(&spin.apply_target) != 1 ||
                    ScanType(closing_type, true) != 1 ||
                    closing_type != "[/spin around]") {
                    std::fprintf(stderr,
                                 "[creature-parser] invalid spin target: %s\n",
                                 filename);
                    return 0;
                }
            } else {
                std::fprintf(stderr,
                             "[creature-parser] unknown special feature %s: %s\n",
                             feature_type.c_str(),
                             filename);
                return 0;
            }

            if (ScanType(closing_type, true) != 1 ||
                closing_type != "[/creature special feature]") {
                std::fprintf(stderr,
                             "[creature-parser] invalid special feature closure: %s\n",
                             filename);
                return 0;
            }
        } else if (type == "[draw new position balloon]") {
            int32_t* const values[] = {
                &script->skill_balloon_position.enabled,
                &script->skill_balloon_position.x,
                &script->skill_balloon_position.y,
                &script->over_skill_balloon_position.enabled,
                &script->over_skill_balloon_position.x,
                &script->over_skill_balloon_position.y
            };
            for (unsigned int index = 0; index < 6; ++index) {
                *values[index] = ScanInt(&success);
                if (!success) {
                    std::fprintf(stderr,
                                 "[creature-parser] short balloon position: %s\n",
                                 filename);
                    LogManager::logFormat(
                        0,
                        "../RDARScriptObject.cpp",
                        "importCreatureScript",
                        0x2674,
                        "creature balloon position loading error : %s",
                        filename);
                    return 0;
                }
            }
            std::string closing_type;
            if (ScanType(closing_type, true) != 1 ||
                closing_type != "[/draw new position balloon]") {
                std::fprintf(stderr,
                             "[creature-parser] invalid balloon closure: %s\n",
                             filename);
                return 0;
            }
        } else if (type == "[etc motion]") {
            if (!ScanClosedStringList(&script->etc_motions,
                                      "[/etc motion]",
                                      filename)) return 0;
        } else if (type == "[random motion]") {
            if (!ScanClosedStringList(&script->random_motions,
                                      "[/random motion]",
                                      filename)) return 0;
        } else if (type == "[attack info]") {
            if (!ScanClosedStringList(&script->attack_infos,
                                      "[/attack info]",
                                      filename)) return 0;
        } else if (type == "[skill string]") {
            if (!ScanClosedStringList(&script->skill_strings,
                                      "[/skill string]",
                                      filename)) return 0;
        } else if (type == "[skill level values]" ||
                   type == "[over skill level values]" ||
                   type == "[piercing]" || type == "[int data]" ||
                   type == "[event evolute creature]") {
            std::vector<int>* values = 0;
            const char* closing_tag = 0;
            if (type == "[skill level values]") {
                values = &script->skill_level_values;
                closing_tag = "[/skill level values]";
            } else if (type == "[over skill level values]") {
                values = &script->over_skill_level_values;
                closing_tag = "[/over skill level values]";
            } else if (type == "[piercing]") {
                values = &script->piercing;
                closing_tag = "[/piercing]";
            } else if (type == "[int data]") {
                values = &script->int_data;
                closing_tag = "[/int data]";
            } else {
                values = &script->event_evolute_creatures;
                closing_tag = "[/event evolute creature]";
            }
            if (!ScanClosedIntList(values, closing_tag, filename)) return 0;
        } else if (type == "[string data]") {
            if (!ScanClosedStringList(&script->string_data,
                                      "[/string data]",
                                      filename)) return 0;
        } else if (type == "[evolution creature multy id]") {
            std::vector<int>* values = &script->evolution_creature_ids;
            values->clear();
            const int count = ScanInt(&success);
            if (!success || count < 0 || count > 4096) return 0;
            values->reserve(static_cast<size_t>(count));
            for (int index = 0; index < count; ++index) {
                value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values->push_back(value);
            }
        } else if (type == "[evolution quest]") {
            script->evolution_quest = ScanInt(static_cast<bool*>(0));
        } else if (type == "[evolution creature id]") {
            script->evolution_creature_id = ScanInt(static_cast<bool*>(0));
        } else if (type == "[evolution level]") {
            script->evolution_level = ScanInt(static_cast<bool*>(0));
        } else if (type == "[selective grow creature]") {
            selectiveGrowCreatureInfo value_info;
            bool closed = false;
            while (!closed) {
                if (ScanType(type, true) != 1) return 0;
                if (type == "[/selective grow creature]") {
                    script->selective_grow_info = value_info;
                    closed = true;
                    continue;
                }
                if (type == "[item count for levelup]") {
                    value_info.item_count_for_levelup = ScanInt(&success);
                } else if (type == "[using exp item]") {
                    value_info.using_exp_item = ScanInt(&success) != 0;
                } else if (type == "[grow Atype item index]") {
                    value_info.grow_a_type_item_index = ScanInt(&success);
                } else if (type == "[grow Btype item index]") {
                    value_info.grow_b_type_item_index = ScanInt(&success);
                } else if (type == "[grow Atype creature id]") {
                    value_info.grow_a_type_creature_id = ScanInt(&success);
                } else if (type == "[grow Btype creature id]") {
                    value_info.grow_b_type_creature_id = ScanInt(&success);
                } else if (type == "[grow level]") {
                    value_info.grow_level = ScanInt(&success);
                } else {
                    std::fprintf(stderr,
                                 "[creature-parser] unknown selective-grow "
                                 "field %s: %s\n",
                                 type.c_str(),
                                 filename);
                    return 0;
                }
                if (!success) return 0;
            }
        } else if (type == "[item evolution]") {
            value = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(0,
                                      "../RDARScriptObject.cpp",
                                      "importCreatureScript",
                                      0x259E,
                                      "creature script loading  %s, evolution_item_id_",
                                      filename);
                return 0;
            }
            script->evolution_item_id = value;
            std::vector<st_crature_id_item_id>* values =
                &script->item_evolution_creature_ids;
            values->clear();
            while (1) {
                st_crature_id_item_id item;
                item.creature_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                item.item_id = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptObject.cpp",
                        "importCreatureScript",
                        0x25AF,
                        "creature script loading  %s, item_evolution_creature_id_ size : %d",
                        filename,
                        static_cast<int>(values->size()));
                    return 0;
                }
                if (item.creature_id == -1) {
                    break;
                }
                values->push_back(item);
            }
            if (ScanType(type, true) != 1 || type != "[/item evolution]") {
                return 0;
            }
        } else if (type == "[skill info]") {
            ScanStr(&script->skill_info);
        } else if (type == "[over skill info]") {
            ScanStr(&script->over_skill_info);
        } else if (type == "[using random skill]") {
            value = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(0,
                                      "../RDARScriptObject.cpp",
                                      "importCreatureScript",
                                      0x266C,
                                      "creature script loading  %s, evolution_item_id_",
                                      filename);
                return 0;
            }
            script->using_random_skill = value != 0;
        } else if (type == "[skill list]") {
            bool in_skill_list = true;
            while (in_skill_list) {
                if (ScanType(type, true) != 1) {
                    std::fprintf(stderr,
                                 "[creature-parser] unterminated skill list: "
                                 "%s\n",
                                 filename);
                    return 0;
                }
                CreatureScriptSkillInfo info;
                int skill_id = -1;
                int is_over_skill = -1;
                if (type == "[/skill list]") {
                    in_skill_list = false;
                } else if (type == "[skill node]") {
                    info.clear();
                    skill_id = ScanInt(&success);
                    if (!success || skill_id < 0) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptObject.cpp",
                            "importCreatureScript",
                            0x25EA,
                            "creature script loading  %s, evolution_item_id_",
                            filename);
                        return 0;
                    }
                    bool in_node = true;
                    bool has_skill_type = false;
                    bool has_preload_passive_indices = false;
                    while (in_node) {
                        if (ScanType(type, true) != 1) {
                            std::fprintf(stderr,
                                         "[creature-parser] unterminated skill "
                                         "node id=%d: %s\n",
                                         skill_id,
                                         filename);
                            return 0;
                        }
                        if (type == "[/skill node]") {
                            in_node = false;
                            if (skill_id < 0 || !has_skill_type ||
                                is_over_skill == -1) {
                                LogManager::logFormat(
                                    0,
                                    "../RDARScriptObject.cpp",
                                    "importCreatureScript",
                                    0x25FC,
                                    "creature script loading  %s, evolution_item_id_",
                                    filename);
                                return 0;
                            }
                            std::map<int, CreatureScriptSkillInfo>* values =
                                is_over_skill ? &script->over_skill_infos
                                              : &script->skill_infos;
                            const std::pair<
                                std::map<int, CreatureScriptSkillInfo>::iterator,
                                bool> inserted =
                                values->insert(std::make_pair(skill_id, info));
                            if (!inserted.second) {
                                std::fprintf(stderr,
                                             "[creature-parser] duplicate %s "
                                             "skill id=%d: %s\n",
                                             is_over_skill ? "over" : "general",
                                             skill_id,
                                             filename);
                                return 0;
                            }
                        } else if (type == "[skill type]") {
                            if (has_skill_type || ScanStr(&text) != 1) return 0;
                            if (text == "general skill") {
                                is_over_skill = 0;
                            } else if (text == "over skill") {
                                is_over_skill = 1;
                            } else {
                                std::fprintf(stderr,
                                             "[creature-parser] unknown skill "
                                             "type %s: %s\n",
                                             text.c_str(),
                                             filename);
                                return 0;
                            }
                            has_skill_type = true;
                        } else if (type == "[skill name]") {
                            if (ScanStr(&info.skill_name) != 1) return 0;
                        } else if (type == "[skill explain]") {
                            if (ScanStr(&info.skill_explain) != 1) return 0;
                        } else if (type == "[skill recovery time]") {
                            info.skill_recovery_time = ScanInt(&success);
                            if (!success) return 0;
                        } else if (type == "[learn skill level]") {
                            info.learn_skill_level = ScanInt(&success);
                            if (!success) return 0;
                        } else if (type == "[skill MP]" ||
                                   type == "[skill mp]") {
                            info.skill_mp = ScanInt(&success);
                            if (!success) return 0;
                        } else if (type == "[skill motion]") {
                            if (ScanStr(&info.skill_motion) != 1) return 0;
                        } else if (type == "[skill under effect]") {
                            if (ScanStr(&info.skill_under_effect) != 1) return 0;
                        } else if (type == "[skill over effect]") {
                            if (ScanStr(&info.skill_over_effect) != 1) return 0;
                        } else if (type == "[skill info]") {
                            if (ScanStr(&info.skill_info) != 1) return 0;
                        } else if (type == "[attack info]") {
                            if (ScanStr(&info.attack_info) != 1) return 0;
                        } else if (type == "[preload Passive Index]" ||
                                   type == "[preload passive index]") {
                            if (has_preload_passive_indices) return 0;
                            const int count = ScanInt(&success);
                            if (!success || count < 0 || count > 4096) {
                                return 0;
                            }
                            info.preload_passive_indices.clear();
                            info.preload_passive_indices.reserve(
                                static_cast<size_t>(count));
                            for (int index = 0; index < count; ++index) {
                                const int passive_index = ScanInt(&success);
                                if (!success || passive_index <= 0) {
                                    return 0;
                                }
                                info.preload_passive_indices.push_back(
                                    passive_index);
                            }
                            has_preload_passive_indices = true;
                        } else {
                            std::fprintf(stderr,
                                         "[creature-parser] unknown skill node "
                                         "field %s id=%d: %s\n",
                                         type.c_str(),
                                         skill_id,
                                         filename);
                            return 0;
                        }
                    }
                } else {
                    std::fprintf(stderr,
                                 "[creature-parser] unexpected skill-list "
                                 "field %s: %s\n",
                                 type.c_str(),
                                 filename);
                    return 0;
                }
            }
        } else {
            // IDA 0x8A382BB logs the unrecognised top-level tag before
            // rejecting the script.  Keep this failure visible; returning
            // silently here obscures the exact missing importer ABI.
            LogManager::logFormat(0,
                                  "../RDARScriptObject.cpp",
                                  "importCreatureScript",
                                  0x2674,
                                  "creature script loading error : %s : %s",
                                  type.c_str(),
                                  filename);
            std::fprintf(stderr,
                         "[creature-parser] unknown tag %s: %s\n",
                         type.c_str(),
                         filename);
            return 0;
        }
    }

    script->index = g_creatureScriptFileList.FindIndexByFullScan(filename);
    return 1;
}

namespace user_creature {

// IDA_EA: 0x8336BD0
// IDA_NAME: _ZN13user_creature28GetInstanceCreatureScriptMgrEv
// DEMANGLED: user_creature::GetInstanceCreatureScriptMgr(void)
CCreatureScriptMgr* GetInstanceCreatureScriptMgr() {
    static CCreatureScriptMgr instance;
    return &instance;
}

// IDA_EA: 0x833C77E
// IDA_NAME: _ZN13user_creature15CCreatureScriptC2Ev
// DEMANGLED: user_creature::CCreatureScript::CCreatureScript(void)
CCreatureScript::CCreatureScript()
    : evolution_creature_ids(),
      item_evolution_creature_ids(),
      event_evolute_creatures() {
    skill_recovery_time = 0;
    over_skill_recovery_time = 0;
    learn_overskill_level = 0;
    artifact_slots = 0;
    evolution_creature_ids.clear();
    event_evolute_creatures.clear();
}

// IDA_EA: 0x833C848
// IDA_NAME: _ZN13user_creature15CCreatureScriptD2Ev
// DEMANGLED: user_creature::CCreatureScript::~CCreatureScript()
CCreatureScript::~CCreatureScript() {
    delete[] artifact_slots;
    artifact_slots = 0;
}

// IDA_EA: 0x833C8E8
// IDA_NAME: _ZN13user_creature15CCreatureScript3SetEP14CreatureScript
// DEMANGLED: user_creature::CCreatureScript::Set(CreatureScript *)
CCreatureScript* CCreatureScript::Set(CreatureScript* script) {
    artifact_slots = new unsigned char[3];
    for (int i = 0; i <= 2; ++i) {
        artifact_slots[i] = script->artifact_slot[i] != 0;
    }

    evolution_quest = script->evolution_quest;
    evolution_creature_id = script->evolution_creature_id;
    evolution_level = script->evolution_level;
    max_level = script->max_level;
    skill_recovery_time = script->skill_recovery_time;
    over_skill_recovery_time = script->over_skill_recovery_time;
    learn_overskill_level =
        static_cast<unsigned char>(script->learn_overskill_level);

    for (std::vector<int>::const_iterator it =
             script->evolution_creature_ids.begin();
         it != script->evolution_creature_ids.end();
         ++it) {
        evolution_creature_ids.push_back(*it);
    }

    for (std::vector<st_crature_id_item_id>::const_iterator it =
             script->item_evolution_creature_ids.begin();
         it != script->item_evolution_creature_ids.end();
         ++it) {
        item_evolution_creature_ids.insert(it->creature_id);
    }

    using_exp_item = script->selective_grow_info.using_exp_item;
    type_select_evolution_level = script->selective_grow_info.grow_level;
    a_type_creature = script->selective_grow_info.grow_a_type_creature_id;
    b_type_creature = script->selective_grow_info.grow_b_type_creature_id;
    a_type_item_index = script->selective_grow_info.grow_a_type_item_index;
    b_type_item_index = script->selective_grow_info.grow_b_type_item_index;
    item_count_for_levelup =
        script->selective_grow_info.item_count_for_levelup;
    event_evolute_creatures = script->event_evolute_creatures;
    return this;
}

// IDA_EA: 0x833CAD8
// IDA_NAME: _ZN13user_creature15CCreatureScript26GetItemEvolutionCreatureIdEi
// DEMANGLED: user_creature::CCreatureScript::GetItemEvolutionCreatureId(int)
int CCreatureScript::GetItemEvolutionCreatureId(int creature_id) {
    return item_evolution_creature_ids.find(creature_id) ==
                   item_evolution_creature_ids.end()
               ? 0
               : creature_id;
}

// IDA_EA: 0x833CB38
// IDA_NAME: _ZN13user_creature15CCreatureScript23IsAvailableArtifactSlotEi
// DEMANGLED: user_creature::CCreatureScript::IsAvailableArtifactSlot(int)
int CCreatureScript::IsAvailableArtifactSlot(int slot_type) {
    if (slot_type >= oldsrv::config::kEquipmentTypeArtifactRed &&
        slot_type <= oldsrv::config::kEquipmentTypeArtifactGreen) {
        return artifact_slots[
            slot_type - oldsrv::config::kEquipmentTypeArtifactRed];
    }
    return 0;
}

// IDA_EA: 0x833CB60
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgrC2Ev
// DEMANGLED: user_creature::CCreatureScriptMgr::CCreatureScriptMgr(void)
CCreatureScriptMgr::CCreatureScriptMgr()
    : creature_id_to_item_id(), item_id_to_script() {}

// IDA_EA: 0x833CBA6
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgr22LoadCreatureScriptFileEv
// DEMANGLED: user_creature::CCreatureScriptMgr::LoadCreatureScriptFile(void)
int CCreatureScriptMgr::LoadCreatureScriptFile() {
    CreatureScriptCacheLoadGuard cache_load;
    CreatureScript local_script;
    AttackInfo attack_info;
    char attack_info_text[256] = {};
    char over_attack_info_text[256] = {};
    char attack_filename[256] = {};
    sync_script::CSyncScript* sync =
        sync_script::GetInstanceSyncScript();

    if (sync->truncate_creature_info_table() != true) {
        std::puts("failed to truncate creature info table");
        return 0;
    }

    for (STScriptFileList::const_iterator it =
             g_creatureScriptFileList.begin();
         it != g_creatureScriptFileList.end();
         ++it) {
        const char* filename = it->second.c_str();
        if (filename == 0) {
            break;
        }
        if (importCreatureScript(&local_script, filename) != 1) {
            // IDA 0x833CD95: this is a persistent error-log record, not a
            // console printf.  It names the exact .cre file that rejected.
            LogManager::logFormat(
                1,
                "Creature.cpp",
                "bool user_creature::CCreatureScriptMgr::LoadCreatureScriptFile()",
                0xF6B,
                "importCreatureScript fail file - %s",
                filename);
            return 0;
        }

        if (sync->IsEnable() != 0) {
            if (local_script.attack_infos.size() != 0 &&
                !local_script.attack_infos[0].empty()) {
                std::sprintf(attack_filename,
                             "%s%s",
                             local_script.script_path.c_str(),
                             local_script.attack_infos[0].c_str());
                if (ImportAttackInfoScript(&attack_info,
                                           attack_filename,
                                           0) != 0) {
                    std::sprintf(attack_info_text,
                                 "%d,%d,%f,%d",
                                 local_script.skill_type,
                                 local_script.skill_mp,
                                 attack_info.damage,
                                 attack_info.attack_type);
                    if (attack_info.elements.size() != 0) {
                        for (std::set<int>::const_iterator element =
                                 attack_info.elements.begin();
                             element != attack_info.elements.end();
                             ++element) {
                            std::sprintf(&attack_info_text[
                                             std::strlen(attack_info_text)],
                                         ",%d",
                                         *element);
                        }
                    } else {
                        std::sprintf(&attack_info_text[
                                         std::strlen(attack_info_text)],
                                     ",%d",
                                     4);
                    }
                } else {
                    std::sprintf(attack_info_text,
                                 "%d,%d",
                                 local_script.skill_type,
                                 local_script.skill_mp);
                }
            }
            if (local_script.attack_infos.size() > 1 &&
                !local_script.attack_infos[1].empty()) {
                std::sprintf(attack_filename,
                             "%s%s",
                             local_script.script_path.c_str(),
                             local_script.attack_infos[1].c_str());
                if (ImportAttackInfoScript(&attack_info,
                                           attack_filename,
                                           0) != 0) {
                    std::sprintf(over_attack_info_text,
                                 "%d,%d,%f,%d",
                                 local_script.over_skill_type,
                                 local_script.over_skill_mp,
                                 attack_info.damage,
                                 attack_info.attack_type);
                    if (attack_info.elements.size() != 0) {
                        for (std::set<int>::const_iterator element =
                                 attack_info.elements.begin();
                             element != attack_info.elements.end();
                             ++element) {
                            std::sprintf(&over_attack_info_text[
                                             std::strlen(over_attack_info_text)],
                                         ",%d",
                                         *element);
                        }
                    } else {
                        std::sprintf(&over_attack_info_text[
                                         std::strlen(over_attack_info_text)],
                                     ",%d",
                                     4);
                    }
                } else {
                    std::sprintf(over_attack_info_text,
                                 "%d,%d",
                                 local_script.over_skill_type,
                                 local_script.over_skill_mp);
                }
            }
        }

        const int creature_id = local_script.index;
        const std::map<int, int>::const_iterator item_it =
            creature_id_to_item_id.find(creature_id);
        if (creature_id < 0 || item_it == creature_id_to_item_id.end() ||
            item_it->second <= 0) {
            LogManager::logFormat(
                1,
                "Creature.cpp",
                "bool user_creature::CCreatureScriptMgr::LoadCreatureScriptFile()",
                0,
                "invalid creature item mapping creature_id=%d file=%s",
                creature_id,
                filename);
            return 0;
        }
        const int item_id = item_it->second;

        if (item_id_to_script.find(creature_id) != item_id_to_script.end()) {
            // The official lookup domain is CreatureScript.index (creature ID),
            // not the equipment item ID stored inside the runtime script.
            cMyTrace duplicate_trace(
                "bool user_creature::CCreatureScriptMgr::LoadCreatureScriptFile()",
                0xFBE,
                5);
            duplicate_trace(
                "[%s][%d]",
                "bool user_creature::CCreatureScriptMgr::LoadCreatureScriptFile()",
                0xFBE);
            return 0;
        }

        if (!CreatureFillPoint::RegisterCreatureScript(item_id,
                                                        local_script)) {
            LogManager::logFormat(
                1,
                "Creature.cpp",
                "bool user_creature::CCreatureScriptMgr::LoadCreatureScriptFile()",
                0,
                "CMD549 fill-point skill import fail file - %s",
                filename);
            return 0;
        }

        CCreatureScript* runtime_script = new CCreatureScript;
        runtime_script->Set(&local_script);
        runtime_script->SetItemId(item_id);

        if (sync->insert_creature_info_to_db(item_id,
                                             &local_script,
                                             attack_info_text,
                                             over_attack_info_text) != 1) {
            std::printf("failed to insert id number < %d > creature info to db\n",
                        local_script.index);
            return 0;
        }

        item_id_to_script.insert(std::make_pair(creature_id, runtime_script));
    }

    cache_load.Commit();
    return 1;
}

// IDA_EA: 0x833D3BC
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgr7DestroyEv
// DEMANGLED: user_creature::CCreatureScriptMgr::Destroy(void)
int CCreatureScriptMgr::Destroy() {
    for (std::map<int, CCreatureScript*>::iterator it =
             item_id_to_script.begin();
         it != item_id_to_script.end();
         ++it) {
        delete it->second;
    }
    item_id_to_script.clear();
    CreatureFillPoint::ResetScriptCache();
    return 1;
}

// IDA_EA: 0x833D49A
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgr18FindCreatureScriptEi
// DEMANGLED: user_creature::CCreatureScriptMgr::FindCreatureScript(int)
CCreatureScript* CCreatureScriptMgr::FindCreatureScript(int item_id) {
    std::map<int, CCreatureScript*>::iterator it =
        item_id_to_script.find(item_id);
    if (it == item_id_to_script.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x833D54A
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgr27RegisterCreatureIdAndItemIdEii
// DEMANGLED: user_creature::CCreatureScriptMgr::RegisterCreatureIdAndItemId(int,int)
int CCreatureScriptMgr::RegisterCreatureIdAndItemId(int creature_id,
                                                    int item_id) {
    if (creature_id_to_item_id.find(creature_id) !=
        creature_id_to_item_id.end()) {
        return 0;
    }
    creature_id_to_item_id.insert(std::make_pair(creature_id, item_id));
    return 1;
}

// IDA_EA: 0x833F4CA
// IDA_NAME: _ZN13user_creature18CCreatureScriptMgrD2Ev
// DEMANGLED: user_creature::CCreatureScriptMgr::~CCreatureScriptMgr()
CCreatureScriptMgr::~CCreatureScriptMgr() { Destroy(); }

// IDA_EA: 0x833F11A
// IDA_NAME: _ZN13user_creature15CCreatureScript9SetItemIdEi
// DEMANGLED: user_creature::CCreatureScript::SetItemId(int)
CCreatureScript* CCreatureScript::SetItemId(int item_id) {
    this->item_id = item_id;
    return this;
}

// IDA_EA: 0x833F128
// IDA_NAME: _ZN13user_creature15CCreatureScript9GetItemIdEv
// DEMANGLED: user_creature::CCreatureScript::GetItemId(void)
int CCreatureScript::GetItemId() { return item_id; }

// IDA_EA: 0x833F134
// IDA_NAME: _ZN13user_creature15CCreatureScript21IsExistEvolutionQuestEv
// DEMANGLED: user_creature::CCreatureScript::IsExistEvolutionQuest(void)
bool CCreatureScript::IsExistEvolutionQuest() { return evolution_quest != 0; }

// IDA_EA: 0x833F144
// IDA_NAME: _ZN13user_creature15CCreatureScript22GetEvolutionCreatureIdEv
// DEMANGLED: user_creature::CCreatureScript::GetEvolutionCreatureId(void)
int CCreatureScript::GetEvolutionCreatureId() { return evolution_creature_id; }

// IDA_EA: 0x833F150
// IDA_NAME: _ZN13user_creature15CCreatureScript25validEventEvlouteCreatureEi
// DEMANGLED: user_creature::CCreatureScript::validEventEvlouteCreature(int)
int CCreatureScript::validEventEvlouteCreature(int creature_id) {
    for (std::vector<int>::iterator it = event_evolute_creatures.begin();
         it != event_evolute_creatures.end();
         ++it) {
        if (*it == creature_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x833F1E0
// IDA_NAME: _ZN13user_creature15CCreatureScript20isEvoluationCreatureEi
// DEMANGLED: user_creature::CCreatureScript::isEvoluationCreature(int)
int CCreatureScript::isEvoluationCreature(int creature_id) {
    for (std::vector<int>::iterator it = evolution_creature_ids.begin();
         it != evolution_creature_ids.end();
         ++it) {
        if (*it == creature_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x833F240
// IDA_NAME: _ZN13user_creature15CCreatureScript17GetEvolutionLevelEv
// DEMANGLED: user_creature::CCreatureScript::GetEvolutionLevel(void)
int CCreatureScript::GetEvolutionLevel() { return evolution_level; }

// IDA_EA: 0x833F24C
// IDA_NAME: _ZN13user_creature15CCreatureScript11GetMaxLevelEv
// DEMANGLED: user_creature::CCreatureScript::GetMaxLevel(void)
int CCreatureScript::GetMaxLevel() { return max_level; }

// IDA_EA: 0x833F280
// IDA_NAME: _ZN13user_creature15CCreatureScript12validItemExpEi
// DEMANGLED: user_creature::CCreatureScript::validItemExp(int)
int CCreatureScript::validItemExp(int item_id) {
    if (using_exp_item != true) {
        return 0;
    }
    return a_type_item_index == item_id || b_type_item_index == item_id;
}

// IDA_EA: 0x833F2BC
// IDA_NAME: _ZN13user_creature15CCreatureScript15validItem_ATypeEi
// DEMANGLED: user_creature::CCreatureScript::validItem_AType(int)
int CCreatureScript::validItem_AType(int item_id) {
    return using_exp_item == true && a_type_item_index == item_id;
}

// IDA_EA: 0x833F2EE
// IDA_NAME: _ZN13user_creature15CCreatureScript15validItem_BTypeEi
// DEMANGLED: user_creature::CCreatureScript::validItem_BType(int)
int CCreatureScript::validItem_BType(int item_id) {
    return using_exp_item == true && b_type_item_index == item_id;
}

// IDA_EA: 0x833F320
// IDA_NAME: _ZN13user_creature15CCreatureScript18IsItemGrowCreatureEv
// DEMANGLED: user_creature::CCreatureScript::IsItemGrowCreature(void)
int CCreatureScript::IsItemGrowCreature() {
    if (a_type_item_index == -1 && b_type_item_index == -1) {
        return 0;
    }
    return using_exp_item;
}

// IDA_EA: 0x833F34A
// IDA_NAME: _ZN13user_creature15CCreatureScript27GetTypeSelectEvolutionLevelEv
// DEMANGLED: user_creature::CCreatureScript::GetTypeSelectEvolutionLevel(void)
int CCreatureScript::GetTypeSelectEvolutionLevel() {
    return type_select_evolution_level;
}

// IDA_EA: 0x833F356
// IDA_NAME: _ZN13user_creature15CCreatureScript26GetGrowItemCountforLevelUpEv
// DEMANGLED: user_creature::CCreatureScript::GetGrowItemCountforLevelUp(void)
int CCreatureScript::GetGrowItemCountforLevelUp() {
    return item_count_for_levelup;
}

// IDA_EA: 0x833F362
// IDA_NAME: _ZN13user_creature15CCreatureScript17GET_ATypeCreatureEv
// DEMANGLED: user_creature::CCreatureScript::GET_ATypeCreature(void)
int CCreatureScript::GET_ATypeCreature() { return a_type_creature; }

// IDA_EA: 0x833F36E
// IDA_NAME: _ZN13user_creature15CCreatureScript17GET_BTypeCreatureEv
// DEMANGLED: user_creature::CCreatureScript::GET_BTypeCreature(void)
int CCreatureScript::GET_BTypeCreature() { return b_type_creature; }

}  // namespace user_creature
