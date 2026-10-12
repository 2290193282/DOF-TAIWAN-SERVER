#include "old_data/PVF/data/sync_script.h"

#include "common/config.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_script.h"
#include "old_data/PVF/creature/datamanager/creature_script.h"
#include "old_data/PVF/data/field_length_checker.h"
#include "old_data/PVF/etc/action_point_sync_script.h"
#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/stackable/stackable_script.h"
#include "old_data/db/core/mysql.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/activity/reward/advance_altar_sync_script_abi.h"
#include "common/types/code_page.h"
#include "common/manager/db_mgr.h"
#include "old_game/economy/equipment/equipment_part_set_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/equipment/old_equipment_stat_info.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_buffer.h"

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <string>

// IDA_EA: 0x861CA1B
// IDA_NAME: _Z22GetSocketTypeCharactert
// DEMANGLED: GetSocketTypeCharacter(unsigned short)
int GetSocketTypeCharacter(uint16_t socket_type) {
    if (socket_type == 8) {
        return 52;
    }
    if (socket_type > 8) {
        switch (socket_type) {
            case 0xffef: return 54;
            case 0xffff: return 55;
            case 0x10: return 53;
            default: break;
        }
    } else {
        switch (socket_type) {
            case 2: return 50;
            case 4: return 51;
            case 1: return 49;
            default: break;
        }
    }
    return 0;
}


namespace sync_script {

namespace {

int MonsterScriptIndex(const STMonsterScript& script) {
    return script.index;
}

const std::string& MonsterScriptName(const STMonsterScript& script) {
    return script.name;
}

const std::string& AICharacterScriptName(
    const STAICharacterCommonScript& script) {
    return script.minimum_info.name;
}

std::string MakeIntList(const int* values, unsigned int count) {
    CStringMaker maker;
    unsigned int index;
    for (index = 0; index != count; ++index) {
        maker(values[index]);
    }
    return maker.c_str();
}

std::string MakeIntList(const std::vector<int>& values) {
    CStringMaker maker;
    std::vector<int>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        maker(*it);
    }
    return maker.c_str();
}

std::string MakeFloatList(const float* values, unsigned int count) {
    CStringMaker maker;
    unsigned int index;
    for (index = 0; index != count; ++index) {
        maker(values[index]);
    }
    return maker.c_str();
}

std::string MakeSecondGrowtypeMaximumLevel(const STSkillScript& script) {
    std::string result;
    int index;
    // df_game_r 0x8623492..0x8623638 serializes the [2],[3] pair from
    // rows 0 through 4 (starts 0x9c, 0xac, 0xbc, 0xcc and 0xdc).
    for (index = 0; index != 5; ++index) {
        if (!result.empty()) {
            result += ";";
        }
        result += MakeIntList(
            script.second_growtype[index].maximum_level, 2);
    }
    return result;
}

std::string MakeSkillIcon(const char* icon_name,
                          int icon_x,
                          const char* icon_sprite,
                          int icon_y) {
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    std::sprintf(buffer, "%s,%d,%s,%d", icon_name, icon_x, icon_sprite,
                 icon_y);
    return buffer;
}

std::string MakeIntPair(int first, int second) {
    char buffer[64];
    std::memset(buffer, 0, sizeof(buffer));
    std::sprintf(buffer, "%d,%d", first, second);
    return buffer;
}

void AppendSqlInteger(CStreamBuffer& stream, int value) {
    stream << ", " << value;
}

void AppendSqlFloat(CStreamBuffer& stream, float value) {
    stream << ", " << value;
}

void AppendSqlZeroFloat(CStreamBuffer& stream) {
    // IDA 0x8620968 emits this literal for an absent module parameter.
    stream << ", 0.0";
}

void AppendSqlText(CStreamBuffer& stream, const char* value) {
    stream << ", '" << value << "'";
}

// IDA 0x8620968 writes the dnf_item_info parameter suffix in this order.
// The member accesses are the typed equivalents of the original fixed offsets.
void AppendStackableModuleParameter(CStreamBuffer& stream,
                                    CSyncScript* sync,
                                    EquipmentParameterInfo* info) {
    int index;
    if (info == NULL) {
        for (index = 0; index != 40; ++index) {
            AppendSqlInteger(stream, 0);
        }
        AppendSqlText(stream, "");
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlZeroFloat(stream);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlInteger(stream, 0);
        AppendSqlZeroFloat(stream);
        AppendSqlInteger(stream, 0);
        AppendSqlText(stream, "");
        return;
    }

    AppendSqlInteger(stream, info->values_000[1].integer);
    AppendSqlInteger(stream, info->values_000[3].integer);
    AppendSqlInteger(stream, info->values_000[5].integer);
    AppendSqlInteger(stream, info->values_000[6].integer);
    AppendSqlInteger(stream, info->values_000[7].integer);
    AppendSqlInteger(stream, info->values_000[9].integer);
    AppendSqlInteger(stream, info->values_000[21].integer);
    AppendSqlInteger(stream, info->values_000[23].integer);
    AppendSqlInteger(stream, info->values_000[25].integer);
    AppendSqlInteger(stream, info->values_000[27].integer);
    for (index = 44; index != 61; ++index) {
        AppendSqlInteger(stream, info->values_000[index].integer);
    }
    AppendSqlInteger(stream, info->values_000[63].integer);
    AppendSqlInteger(stream, info->values_000[64].integer);
    AppendSqlInteger(stream, info->values_000[66].integer);
    AppendSqlInteger(stream, info->values_000[61].integer);
    AppendSqlInteger(stream, info->values_000[62].integer);
    AppendSqlInteger(stream, info->values_000[67].integer);
    AppendSqlInteger(stream, info->values_000[68].integer);
    AppendSqlInteger(stream, info->values_000[69].integer);
    AppendSqlInteger(stream, info->values_000[71].integer);
    for (index = 74; index != 78; ++index) {
        AppendSqlInteger(stream, info->values_000[index].integer);
    }
    AppendSqlText(stream, sync->GetAttackElement(info));
    AppendSqlInteger(stream, info->values_140[1].integer);
    AppendSqlFloat(stream, info->values_140[2].real);
    AppendSqlInteger(stream, info->values_140[3].integer);
    AppendSqlInteger(stream, info->values_220[0].integer);
    AppendSqlInteger(stream, info->values_220[1].integer);
    AppendSqlInteger(stream, info->value_23c);
    AppendSqlInteger(stream, info->value_23d);
    AppendSqlFloat(stream, info->values_000[78].real);
    AppendSqlFloat(stream, info->values_220[2].real);
    AppendSqlInteger(stream, info->value_22c);
    AppendSqlFloat(stream, info->value_274.real);
    AppendSqlFloat(stream, info->values_000[2].real);
    AppendSqlFloat(stream, info->values_000[4].real);
    for (index = 2; index != 7; ++index) {
        AppendSqlInteger(stream, info->values_280[index].integer);
    }
    AppendSqlInteger(stream, info->values_280[1].integer);
    AppendSqlFloat(stream, info->values_000[79].real);
    AppendSqlInteger(stream, info->values_000[29].integer);
    AppendSqlInteger(stream, info->values_000[30].integer);
    AppendSqlFloat(stream, info->values_000[31].real);
    AppendSqlInteger(stream, info->values_000[32].integer);
    AppendSqlInteger(stream, info->values_000[33].integer);
    AppendSqlFloat(stream, info->values_000[34].real);
    AppendSqlInteger(stream, info->values_000[65].integer);

    CStringMaker maker;
    std::vector<LevelUpSkill>::const_iterator it;
    for (it = info->skill_levelups_24c.begin();
         it != info->skill_levelups_24c.end(); ++it) {
        maker(*it);
    }
    AppendSqlText(stream, maker.c_str());
}

// IDA 0x861D87E writes the equipment-specific parameter segment in this
// exact order.  Every access below names the field recorded in the i386 PVF
// layout rather than reintroducing a byte-offset view of EquipmentParameterInfo.
void AppendEquipmentModuleParameter(CStreamBuffer& stream,
                                    CSyncScript* sync,
                                    EquipmentParameterInfo& info) {
    int index;
    AppendSqlInteger(stream, info.values_000[1].integer);
    AppendSqlInteger(stream, info.values_000[3].integer);
    AppendSqlInteger(stream, info.values_000[5].integer);
    AppendSqlInteger(stream, info.values_000[6].integer);
    AppendSqlInteger(stream, info.values_000[7].integer);
    AppendSqlInteger(stream, info.values_000[9].integer);
    AppendSqlInteger(stream, info.values_000[21].integer);
    AppendSqlInteger(stream, info.values_000[23].integer);
    AppendSqlInteger(stream, info.values_000[25].integer);
    AppendSqlInteger(stream, info.values_000[27].integer);
    for (index = 44; index != 61; ++index) {
        AppendSqlInteger(stream, info.values_000[index].integer);
    }
    AppendSqlInteger(stream, info.values_000[63].integer);
    AppendSqlInteger(stream, info.values_000[64].integer);
    AppendSqlInteger(stream, info.values_000[66].integer);
    AppendSqlInteger(stream, info.values_000[61].integer);
    AppendSqlInteger(stream, info.values_000[62].integer);
    AppendSqlInteger(stream, info.values_000[67].integer);
    AppendSqlInteger(stream, info.values_000[68].integer);
    AppendSqlInteger(stream, info.values_000[69].integer);
    AppendSqlInteger(stream, info.values_000[71].integer);
    for (index = 74; index != 78; ++index) {
        AppendSqlInteger(stream, info.values_000[index].integer);
    }
    AppendSqlText(stream, sync->GetAttackElement(&info));
    AppendSqlInteger(stream, info.values_140[1].integer);
    AppendSqlFloat(stream, info.values_140[2].real);
    AppendSqlInteger(stream, info.values_140[3].integer);
    AppendSqlInteger(stream, info.values_220[0].integer);
    AppendSqlInteger(stream, info.values_220[1].integer);
    AppendSqlInteger(stream, info.value_23c);
    AppendSqlInteger(stream, info.value_23d);
    AppendSqlFloat(stream, info.values_000[78].real);
    AppendSqlFloat(stream, info.values_220[2].real);
    AppendSqlInteger(stream, info.value_22c);

    CStringMaker maker;
    std::vector<LevelUpSkill>::const_iterator it;
    for (it = info.skill_levelups_24c.begin();
         it != info.skill_levelups_24c.end(); ++it) {
        maker(*it);
    }
    AppendSqlText(stream, maker.c_str());
}

std::string MakeItemAuraList(const std::vector<ItemAuraInfo>& values) {
    CStringMaker maker;
    std::vector<ItemAuraInfo>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        maker(*it);
    }
    return maker.c_str();
}

std::string MakeSkillDataUpList(
    const std::vector<EquipmentSkillDataUp>& values) {
    CStringMaker maker;
    std::vector<EquipmentSkillDataUp>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        maker(*it);
    }
    return maker.c_str();
}

void AppendAniVariation(char* output,
                        bool has_previous,
                        int character_job,
                        int first_value,
                        int second_value,
                        const std::vector<STEquipmentAniScript>& animations) {
    char buffer[256];
    char compact[256];
    char number[16];
    std::memset(output, 0, 256);
    std::sprintf(output, "%02d%02d", first_value, second_value);
    if (has_previous) {
        std::memset(number, 0, sizeof(number));
        std::sprintf(number, "*%d", character_job);
        std::strcat(output, number);
    }
    std::memset(number, 0, sizeof(number));
    std::sprintf(number, "*%d", static_cast<int>(animations.size()));
    std::strcat(output, number);
    std::vector<STEquipmentAniScript>::const_iterator it;
    for (it = animations.begin(); it != animations.end(); ++it) {
        std::memset(buffer, 0, sizeof(buffer));
        std::memset(compact, 0, sizeof(compact));
        std::strcpy(buffer, it->reserved_motion.c_str());
        int target = 0;
        for (unsigned int index = 0; index < std::strlen(buffer); ++index) {
            if (buffer[index] != '_') {
                compact[target++] = buffer[index];
            }
        }
        std::strcat(output, "*");
        std::strcat(output, compact);
    }
}

void MakeEquipmentAnimations(const STEquipmentScript& script,
                             char normal[256],
                             char expanded[256],
                             char normal_second[256],
                             char expanded_second[256]) {
    std::memset(normal, 0, 256);
    std::memset(expanded, 0, 256);
    std::memset(normal_second, 0, 256);
    std::memset(expanded_second, 0, 256);
    bool has_previous = false;
    for (int character_job = 0;
         character_job < oldsrv::config::kCharacterJobCount;
         ++character_job) {
        if (script.item.usable_job_flags[character_job] == 0) {
            continue;
        }
        EquipmentAniInfoScript* info =
            const_cast<STEquipmentScript*>(&script)->getAniInfo(
                static_cast<ENUM_CHARACTERJOB>(character_job));
        if (info == NULL) {
            break;
        }
        if (info->unknown_30 == -1 && info->unknown_34 == -1 &&
            info->unknown_0C == -1 && info->unknown_10 == -1) {
            continue;
        }
        if (info->unknown_14 != 0) {
            if (info->unknown_30 == -1 && info->unknown_34 == -1) {
                continue;
            }
            char variation[256];
            AppendAniVariation(variation, has_previous, character_job,
                               info->unknown_30, info->unknown_34,
                               info->layer_ani);
            if (has_previous) {
                std::strcpy(expanded_second, variation);
            } else {
                std::strcpy(expanded, variation);
                has_previous = true;
            }
        } else if (info->unknown_0C != -1 || info->unknown_10 != -1) {
            char variation[256];
            AppendAniVariation(variation, has_previous, character_job,
                               info->unknown_0C, info->unknown_10,
                               info->base_ani);
            if (has_previous) {
                std::strcpy(normal_second, variation);
            } else {
                std::strcpy(normal, variation);
                has_previous = true;
            }
        }
    }
}

}  // namespace

// IDA_EA: 0x8626AE2
// IDA_NAME: _ZN11sync_script11CSyncScript11SWEAPONTYPEC2E17ENUM_CHARACTERJOBi
// DEMANGLED: sync_script::CSyncScript::SWEAPONTYPE::SWEAPONTYPE(ENUM_CHARACTERJOB,int)
CSyncScript::SWEAPONTYPE::SWEAPONTYPE(ENUM_CHARACTERJOB character_job_value,
                                      int weapon_type_value)
    : character_job(character_job_value), weapon_type(weapon_type_value) {}

// IDA_EA: 0x8626AF8
// IDA_NAME: _ZNK11sync_script11CSyncScript11SWEAPONTYPEltERKS1_
// DEMANGLED: sync_script::CSyncScript::SWEAPONTYPE::operator<(sync_script::CSyncScript::SWEAPONTYPE const&)const
bool CSyncScript::SWEAPONTYPE::operator<(
    const CSyncScript::SWEAPONTYPE& other) const {
    if (character_job < other.character_job) {
        return true;
    }
    return character_job == other.character_job &&
           weapon_type < other.weapon_type;
}

// IDA_EA: 0x8626B3E
// IDA_NAME: _ZN11sync_script11CSyncScript10SARMORTYPEC2EhSs
// DEMANGLED: sync_script::CSyncScript::SARMORTYPE::SARMORTYPE(unsigned char,std::string)
CSyncScript::SARMORTYPE::SARMORTYPE(uint8_t master_type_value,
                                    std::string armor_name_value)
    : master_type(master_type_value), armor_name(armor_name_value) {}

// IDA_EA: 0x86299F4
// IDA_NAME: _ZN11sync_script11CSyncScript10SARMORTYPEC2ERKS1_
// DEMANGLED: sync_script::CSyncScript::SARMORTYPE::SARMORTYPE(sync_script::CSyncScript::SARMORTYPE const&)
CSyncScript::SARMORTYPE::SARMORTYPE(const CSyncScript::SARMORTYPE& other)
    : master_type(other.master_type), armor_name(other.armor_name) {}

// IDA_EA: 0x8627C7E
// IDA_NAME: _ZN11sync_script11CSyncScript10SARMORTYPED2Ev
// DEMANGLED: sync_script::CSyncScript::SARMORTYPE::~SARMORTYPE()
CSyncScript::SARMORTYPE::~SARMORTYPE() {}

// IDA_EA: 0x8626BA0
// IDA_NAME: _ZNK11sync_script11CSyncScript10SARMORTYPEltERKS1_
// DEMANGLED: sync_script::CSyncScript::SARMORTYPE::operator<(sync_script::CSyncScript::SARMORTYPE const&)const
bool CSyncScript::SARMORTYPE::operator<(
    const CSyncScript::SARMORTYPE& other) const {
    if (master_type < other.master_type) {
        return true;
    }
    return master_type == other.master_type &&
           armor_name.compare(other.armor_name) < 0;
}

// IDA_EA: 0x8626C08
// IDA_NAME: _ZN11sync_script11CSyncScript9SITEMTYPEC2Ev
// DEMANGLED: sync_script::CSyncScript::SITEMTYPE::SITEMTYPE(void)
CSyncScript::SITEMTYPE::SITEMTYPE()
    : item_type(0), master_type(0), sub_type(0), sub_no(0), description() {}

// IDA_EA: 0x8626C3C
// IDA_NAME: _ZN11sync_script11CSyncScript9SITEMTYPEC2EbhhtSs
// DEMANGLED: sync_script::CSyncScript::SITEMTYPE::SITEMTYPE(bool,unsigned char,unsigned char,unsigned short,std::string)
CSyncScript::SITEMTYPE::SITEMTYPE(bool item_type_value,
                                  uint8_t master_type_value,
                                  uint8_t sub_type_value,
                                  uint16_t sub_no_value,
                                  std::string description_value)
    : item_type(item_type_value ? 1 : 0),
      master_type(master_type_value),
      sub_type(sub_type_value),
      sub_no(sub_no_value),
      description(description_value) {}

// IDA_EA: 0x8627BBE
// IDA_NAME: _ZN11sync_script11CSyncScript9SITEMTYPEC2ERKS1_
// DEMANGLED: sync_script::CSyncScript::SITEMTYPE::SITEMTYPE(sync_script::CSyncScript::SITEMTYPE const&)
CSyncScript::SITEMTYPE::SITEMTYPE(const CSyncScript::SITEMTYPE& other)
    : item_type(other.item_type),
      master_type(other.master_type),
      sub_type(other.sub_type),
      sub_no(other.sub_no),
      description(other.description) {}

// IDA_EA: 0x8627C12
// IDA_NAME: _ZN11sync_script11CSyncScript9SITEMTYPED2Ev
// DEMANGLED: sync_script::CSyncScript::SITEMTYPE::~SITEMTYPE()
CSyncScript::SITEMTYPE::~SITEMTYPE() {}

// IDA_EA: 0x8627C28
// IDA_NAME: _ZN11sync_script11CSyncScript9SITEMTYPEaSERKS1_
// DEMANGLED: sync_script::CSyncScript::SITEMTYPE::operator=(sync_script::CSyncScript::SITEMTYPE const&)
CSyncScript::SITEMTYPE& CSyncScript::SITEMTYPE::operator=(
    const CSyncScript::SITEMTYPE& other) {
    item_type = other.item_type;
    master_type = other.master_type;
    sub_type = other.sub_type;
    sub_no = other.sub_no;
    description = other.description;
    return *this;
}

// IDA_EA: 0x861D87E
// IDA_NAME: _ZN11sync_script11CSyncScript28insert_equiptment_item_to_dbEP17STEquipmentScriptjii
// DEMANGLED: sync_script::CSyncScript::insert_equiptment_item_to_db(STEquipmentScript *,unsigned int,int,int)
bool CSyncScript::insert_equiptment_item_to_db(STEquipmentScript* script,
                                               unsigned int item_id,
                                               int sync_type,
                                               int db_piece_count) {
    // IDA @0x861D8C1..0x861D904: n10 is assigned only for the first nonzero
    // usable-job flag. The original has no no-job fallback before
    // sync_type == 0 passes it to GetType, so do not replace this with a
    // synthetic default job merely to silence -Wmaybe-uninitialized.
    ENUM_CHARACTERJOB character_job;
    int set_item_id = 0;
    int master_type = 0;
    int sub_type = 0;
    int sql_item_id = static_cast<int>(item_id);
    int index;

    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    for (index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        if (script->item.usable_job_flags[index] != 0) {
            character_job = static_cast<ENUM_CHARACTERJOB>(index);
            break;
        }
    }

    switch (sync_type) {
        case 1:
            set_item_id = sql_item_id;
            sql_item_id = -1;
            break;
        case 2:
            set_item_id = sql_item_id;
            sql_item_id = -2;
            break;
        case 0:
            if (GetType(script->item.strings_ac[4], character_job,
                        script->values_1b4[1], master_type, sub_type,
                        true) != 1) {
                return false;
            }
            sql_item_id = script->item.index;
            for (std::map<int, EquipmentParameterInfo>::iterator it =
                     script->set_parameters_678.begin();
                 it != script->set_parameters_678.end(); ++it) {
                STEquipmentScript parameter_script;
                parameter_script.parameter_1c0 = it->second;
                insert_equiptment_item_to_db(&parameter_script,
                                              static_cast<unsigned int>(sql_item_id),
                                              1,
                                              it->first);
            }
            break;
        default:
            break;
    }

    char jewel_type[6];
    char hide_growtype_avatar[64];
    char normal_ani[256];
    char expand_ani[256];
    char normal_ani_second[256];
    char expand_ani_second[256];
    char extra_icon_indexes[6];
    std::memset(jewel_type, 0, sizeof(jewel_type));
    std::memset(hide_growtype_avatar, 0, sizeof(hide_growtype_avatar));
    std::memset(extra_icon_indexes, 0, sizeof(extra_icon_indexes));

    int jewel_count = 0;
    for (std::vector<unsigned short>::const_iterator it =
             script->avatar_emblems_6cc.begin();
         it != script->avatar_emblems_6cc.end() && jewel_count <= 4;
         ++it) {
        jewel_type[jewel_count++] =
            static_cast<char>(::GetSocketTypeCharacter(*it));
    }
    for (std::vector<stAvatarTypeSelect_t>::const_iterator selection =
             script->avatar_type_select_6c0.begin();
         selection != script->avatar_type_select_6c0.end(); ++selection) {
        if (selection->socket_count > 0) {
            for (int socket_index = 0;
                 socket_index < selection->socket_count && jewel_count <= 4;
                 ++socket_index) {
                jewel_type[jewel_count++] = static_cast<char>(
                    ::GetSocketTypeCharacter(selection->socket_types[socket_index]));
            }
        }
    }

    int hide_index = 0;
    for (std::vector<int>::const_iterator it = script->values_768.begin();
         it != script->values_768.end(); ++it) {
        char one_type[4];
        std::memset(one_type, 0, sizeof(one_type));
        if (hide_index != 0) {
            std::sprintf(one_type, "*%d", *it);
        } else {
            std::sprintf(one_type, "%d", *it);
        }
        std::strcat(hide_growtype_avatar, one_type);
        ++hide_index;
    }

    MakeEquipmentAnimations(*script, normal_ani, expand_ani,
                            normal_ani_second, expand_ani_second);

    int extra_icon_count = 0;
    for (std::vector<int>::const_iterator it = script->values_774.begin();
         it != script->values_774.end() && extra_icon_count <= 4; ++it) {
        extra_icon_indexes[extra_icon_count++] = static_cast<char>(*it + 48);
    }

    const std::string& explain_source =
        script->item.strings_ac[2].empty() ? script->strings_6a4[0]
                                            : script->item.strings_ac[2];
    char converted_name[1024];
    char converted_explain[2196];
    char escaped[11][4096];
    std::memset(converted_name, 0, sizeof(converted_name));
    std::memset(converted_explain, 0, sizeof(converted_explain));
    std::memset(escaped, 0, sizeof(escaped));

    if (CodePage::script2Database(
            const_cast<char*>(script->item.strings_ac[0].c_str()),
            converted_name) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_equiptment_item_to_db(STEquipmentScript*, unsigned int, int, int)",
            920,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->item.strings_ac[0].c_str(),
              converted_name, script->item.index);
        return false;
    }
    if (CodePage::script2Database(const_cast<char*>(explain_source.c_str()),
                                  converted_explain) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_equiptment_item_to_db(STEquipmentScript*, unsigned int, int, int)",
            927,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), explain_source.c_str(), converted_explain,
              script->item.index);
        return false;
    }

    ::GetEscapeString(db_handle, converted_name, escaped[0]);
    ::GetEscapeString(db_handle, script->item.strings_ac[1].c_str(),
                      escaped[1]);
    ::GetEscapeString(db_handle, converted_explain, escaped[2]);

    CStreamBuffer values;
    values << sql_item_id;
    AppendSqlText(values, escaped[0]);
    AppendSqlText(values, escaped[1]);
    AppendSqlText(values, escaped[2]);
    AppendSqlInteger(values, master_type);
    AppendSqlInteger(values, sub_type);
    AppendSqlText(values, GetUsableCharacter(script));
    AppendSqlInteger(values, script->item.grade);
    AppendSqlText(values, GetAttachType(script->item.values_08[0]));
    AppendSqlInteger(values, script->item.values_5c[0]);
    AppendSqlInteger(values, script->values_178[3]);
    AppendSqlInteger(values, script->item.values_08[1]);
    AppendSqlInteger(values, script->item.values_08[2]);
    AppendSqlInteger(values, script->item.value_34);
    AppendSqlInteger(values, script->item.values_08[3]);
    AppendSqlInteger(values, script->item.values_08[4]);
    AppendSqlInteger(values, script->item.values_08[5]);
    AppendSqlInteger(values, script->values_1b4[2]);
    AppendSqlInteger(values, script->item.values_5c[3]);
    AppendEquipmentModuleParameter(values, this, script->parameter_1c0);

    const char* set_type =
        script->set_name_668.empty() && script->values_66c.empty() &&
                script->value_6a0 == -1 && script->value_790 == -1
            ? "n"
            : "y";
    AppendSqlText(values, set_type);
    AppendSqlText(values, jewel_type);
    ::GetEscapeString(db_handle, script->parameter_1c0.value_27c.c_str(),
                      escaped[4]);
    AppendSqlText(values, escaped[4]);
    ::GetEscapeString(db_handle, script->item.strings_ac[3].c_str(),
                      escaped[5]);
    AppendSqlText(values, escaped[5]);
    AppendSqlInteger(values, script->parameter_1c0.values_280[0].integer);
    AppendSqlInteger(values, script->item.value_24);
    AppendSqlInteger(values, script->values_178[3]);
    values << ", '" << script->item.value_28 << "," << script->item.value_2c
           << "'";
    AppendSqlInteger(values, script->parameter_1c0.values_000[29].integer);
    AppendSqlInteger(values, script->parameter_1c0.values_000[30].integer);
    AppendSqlFloat(values, script->parameter_1c0.values_000[31].real);
    AppendSqlInteger(values, script->parameter_1c0.values_000[32].integer);
    AppendSqlInteger(values, script->parameter_1c0.values_000[33].integer);
    AppendSqlFloat(values, script->parameter_1c0.values_000[34].real);
    for (index = 2; index != 7; ++index) {
        AppendSqlInteger(values,
                          script->parameter_1c0.values_280[index].integer);
    }
    AppendSqlInteger(values, script->parameter_1c0.values_000[59].integer);
    AppendSqlInteger(values, script->parameter_1c0.values_000[65].integer);
    AppendSqlInteger(values, script->parameter_1c0.values_000[66].integer);
    AppendSqlInteger(values, script->parameter_1c0.values_280[1].integer);
    const std::string item_aura =
        MakeItemAuraList(script->parameter_1c0.aura_infos_188);
    AppendSqlText(values, item_aura.c_str());
    AppendSqlFloat(values, script->parameter_1c0.values_000[79].real);
    ::GetEscapeString(db_handle, script->set_name_668.c_str(), escaped[6]);
    AppendSqlText(values, escaped[6]);
    const std::string set_item = MakeIntList(script->values_66c);
    AppendSqlText(values, set_item.c_str());
    ::GetEscapeString(db_handle, script->strings_6a4[3].c_str(), escaped[7]);
    AppendSqlText(values, escaped[7]);
    ::GetEscapeString(db_handle, script->strings_6a4[4].c_str(), escaped[8]);
    AppendSqlText(values, escaped[8]);
    ::GetEscapeString(db_handle, script->parameter_1c0.value_278.c_str(),
                      escaped[9]);
    AppendSqlText(values, escaped[9]);
    ::GetEscapeString(db_handle, script->parameter_1c0.value_27c.c_str(),
                      escaped[10]);
    AppendSqlText(values, escaped[10]);
    AppendSqlInteger(values, script->value_790);
    AppendSqlText(values, hide_growtype_avatar);
    const std::string skill_data_up =
        MakeSkillDataUpList(script->parameter_1c0.skill_data_up_1b8);
    AppendSqlText(values, skill_data_up.c_str());
    AppendSqlInteger(values, set_item_id);
    AppendSqlInteger(values, db_piece_count);
    AppendSqlInteger(values, script->value_6a0);
    AppendSqlText(values, normal_ani);
    AppendSqlText(values, expand_ani);
    AppendSqlText(values, normal_ani_second);
    AppendSqlText(values, expand_ani_second);
    AppendSqlText(values, extra_icon_indexes);
    AppendSqlFloat(values, script->parameter_1c0.value_274.real);
    const char* url = GetURL(script->item.icon.c_str(),
                             script->item.icon.length(),
                             script->item.icon_index);
    ::GetEscapeString(db_handle, url, escaped[3]);
    AppendSqlText(values, escaped[3]);
    AppendSqlInteger(values, script->item.values_c4[0]);
    const std::string extra_icons = MakeIntList(script->values_774);
    AppendSqlText(values, extra_icons.c_str());
    AppendSqlFloat(values, script->parameter_1c0.values_000[2].real);
    AppendSqlFloat(values, script->parameter_1c0.values_000[4].real);

    db_handle->set_query(
        "inSert into dnf_item_info(it_no, it_name, it_eng_name, it_explain, master_type, sub_type, job, class, revert, level, skill, create_ratio, rarity, weight, price, cash, medal, durability, cooltime, hp_max, mp_max, phy_att, phy_def, mag_att, mag_def, equip_phy_att, equip_phy_def, equip_mag_att, equip_mag_def, ref_fire, ref_water, ref_dark, ref_light, ref_all, ref_slow, ref_freeze, ref_poison, ref_stun, ref_cus, ref_blind, ref_lite, ref_ston, ref_sleep, ref_deekement, ref_deadlystrike, ref_bleeding, ref_confuse, ref_hold, ref_all_stat, ref_pierce, ref_stuck, inven_max, hp_regenrate, mp_regenrate, mov_speed, att_speed, quest, hit_recovery, jump, att_element, att_active_status, att_active_status_ratio, att_active_status_pow, att_backforce, att_upforce, att_hp_drain, att_mp_drain, criticalhit_rate, stuck_rate, att_defenseIgnore, skill_levelup, set_type, jewel_type, detail_explain, flavor_text, anti_evil, value, required_skill, need_material, physical_absolute_damage, physical_damage_reduce, physical_absolute_defense, magical_absolute_damage, magical_damage_reduce, magical_absolute_defense, fire_attack, water_attack, dark_attack, light_attack, all_attack, weapon_break_resistance, armor_break_resistance, all_activestatus_resistance, rigidity, item_aura, magical_critical_hit, set_name, set_item, fullset_basic_explain, fullset_detail_explain, parameter_basic_explain, parameter_detail_explain, part_set_index, hide_equipment, skill_data_up, it_set_no, db_piece_count, set_item_master, ani_variation, ani_variation_expand, ani_variation2, ani_variation_expand2, hide_growtype_avatar, room_list_move_speed_rate, url, icon_mark_number, extra_icon_idx_list, hp_max_rate, mp_max_rate) values(%s)",
        values.c_str());
    CFieldLengthChecker checker;
    checker.AddField(db_handle->GetQueryString());
    if (!db_handle->exec(true)) {
        return false;
    }

    const std::vector<STAvatarAbilityVariation>& avatar_abilities =
        script->parameter_1c0.avatar_select_abilities_264;
    for (size_t ability_no = 0u;
         ability_no < avatar_abilities.size();
         ++ability_no) {
        const STAvatarAbilityVariation* const ability =
            &avatar_abilities[ability_no];
        if (ability->ability_index < 0) {
            continue;
        }
        const int ability_job =
            ability->level_up_skill.skill_type ==
                    oldsrv::config::kCharacterJobCommon
                ? 99
                : ability->level_up_skill.skill_type;
        db_handle->set_query(
            "inSert into avatar_select_ability(it_no, ability_no, ability_type, rate_change, value, job, skill_index, skill_level) values(%d,%d,%d,%d,%f,%d,%d,%d)",
            script->item.index,
            static_cast<int>(ability_no),
            ability->ability_index,
            ability->sign_type,
            ability->value,
            ability_job,
            ability->level_up_skill.skill_id,
            ability->level_up_skill.skill_level);
        if (!db_handle->exec(true)) {
            return false;
        }
    }

    if (item_types_["artifact red"].sub_no == sub_type ||
        item_types_["artifact blue"].sub_no == sub_type ||
        item_types_["artifact green"].sub_no == sub_type) {
        insert_artifact_info_to_db(script);
    }
    return true;
}

// IDA_EA: 0x861CC6C
// IDA_NAME: _ZN11sync_script11CSyncScriptC2Ev
// DEMANGLED: sync_script::CSyncScript::CSyncScript(void)
CSyncScript::CSyncScript()
    : type_additions_(),
      reverse_desc_(),
      item_types_(),
      weapon_types_(),
      armor_types_(),
      master_type_names_(),
      material_type_names_() {
    enable_ = 0;
}

// IDA_EA: 0x861CD7A
// IDA_NAME: _ZN11sync_script11CSyncScriptD2Ev
// DEMANGLED: sync_script::CSyncScript::~CSyncScript()
CSyncScript::~CSyncScript() {
    type_additions_.clear();
    reverse_desc_.clear();
    item_types_.clear();
    weapon_types_.clear();
    armor_types_.clear();
    master_type_names_.clear();
    material_type_names_.clear();
}

// IDA_EA: 0x82A71F0
// IDA_NAME: _ZN11sync_script11CSyncScript9SetEnableEb
// DEMANGLED: sync_script::CSyncScript::SetEnable(bool)
CSyncScript* CSyncScript::SetEnable(bool enable) {
    enable_ = enable ? 1 : 0;
    return this;
}

// IDA_EA: 0x82A7208
// IDA_NAME: _ZN11sync_script11CSyncScript8IsEnableEv
// DEMANGLED: sync_script::CSyncScript::IsEnable(void)
int CSyncScript::IsEnable() {
    return enable_;
}

// IDA_EA: 0x8375F58
// IDA_NAME: _ZN11sync_script12CStringMakerC2Ev
// DEMANGLED: sync_script::CStringMaker::CStringMaker(void)
CStringMaker::CStringMaker() : value_() {}

// IDA_EA: 0x837F594
// IDA_NAME: _ZN11sync_script12CStringMakerC2ERKS0_
// DEMANGLED: sync_script::CStringMaker::CStringMaker(sync_script::CStringMaker const&)
CStringMaker::CStringMaker(const CStringMaker& other) : value_(other.value_) {}

// IDA_EA: 0x837F580
// IDA_NAME: _ZN11sync_script12CStringMakerD2Ev
// DEMANGLED: sync_script::CStringMaker::~CStringMaker()
CStringMaker::~CStringMaker() {}

// IDA_EA: 0x8375F6C
// IDA_NAME: _ZN11sync_script12CStringMakerclEi
// DEMANGLED: sync_script::CStringMaker::operator()(int)
CStringMaker& CStringMaker::operator()(int value) {
    char text[256];
    std::memset(text, 0, sizeof(text));
    std::sprintf(text, "%d", value);
    if (!value_.empty()) {
        value_ += ",";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8626CD0
// IDA_NAME: _ZN11sync_script12CStringMakerclEf
// DEMANGLED: sync_script::CStringMaker::operator()(float)
CStringMaker& CStringMaker::operator()(float value) {
    char text[256];
    std::memset(text, 0, sizeof(text));
    std::sprintf(text, "%f", value);
    if (!value_.empty()) {
        value_ += ",";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8626D98
// IDA_NAME: _ZN11sync_script12CStringMakerclE12SkillUseItem
// DEMANGLED: sync_script::CStringMaker::operator()(SkillUseItem)
CStringMaker& CStringMaker::operator()(SkillUseItem value) {
    char text[256];
    std::memset(text, 0, sizeof(text));
    std::sprintf(text, "%d,%d,%d", value.item_index, value.value_04,
                 value.value_08);
    if (!value_.empty()) {
        value_ += ";";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8626E6E
// IDA_NAME: _ZN11sync_script12CStringMakerclE20EquipmentSkillDataUp
// DEMANGLED: sync_script::CStringMaker::operator()(EquipmentSkillDataUp)
CStringMaker& CStringMaker::operator()(EquipmentSkillDataUp value) {
    char text[256];
    std::memset(text, 0, sizeof(text));
    std::sprintf(text, "%d,%d,%d,%d,%d,%d,%d", value.character_job,
                 value.skill_index, value.value_08, value.sign_is_not_plus,
                 value.value_10, value.data_up_type, value.module_type);
    if (!value_.empty()) {
        value_ += ";";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8626F7A
// IDA_NAME: _ZN11sync_script12CStringMakerclE12LevelUpSkill
// DEMANGLED: sync_script::CStringMaker::operator()(LevelUpSkill)
CStringMaker& CStringMaker::operator()(LevelUpSkill value) {
    char text[256];
    std::memset(text, 0, sizeof(text));
    const int character_job =
        value.skill_type == oldsrv::config::kCharacterJobCommon
            ? 99 : value.skill_type;
    std::sprintf(text, "%d,%d,%d", character_job, value.skill_id,
                 value.skill_level);
    if (!value_.empty()) {
        value_ += ";";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8627066
// IDA_NAME: _ZN11sync_script12CStringMakerclE12ItemAuraInfo
// DEMANGLED: sync_script::CStringMaker::operator()(ItemAuraInfo)
CStringMaker& CStringMaker::operator()(ItemAuraInfo value) {
    char text[256];
    int value_bits = 0;
    std::memset(text, 0, sizeof(text));
    std::memcpy(&value_bits, &value.value, sizeof(value_bits));
    std::sprintf(text, "%d,%d,%d,%d", value.active_status_type,
                 value.use_percent, value_bits, value.duration);
    if (!value_.empty()) {
        value_ += ";";
    }
    value_ += text;
    return *this;
}

// IDA_EA: 0x8627146
// IDA_NAME: _ZN11sync_script12CStringMakerclERSt6vectorIfSaIfEE
// DEMANGLED: sync_script::CStringMaker::operator()(std::vector<float,std::allocator<float>> &)
CStringMaker& CStringMaker::operator()(std::vector<float>& values) {
    if (values.empty()) {
        return *this;
    }
    if (!value_.empty()) {
        value_ += ";";
    }
    char text[256];
    for (std::vector<float>::iterator it = values.begin(); it != values.end();
         ++it) {
        std::memset(text, 0, sizeof(text));
        std::sprintf(text, "%f", *it);
        if (it != values.begin()) {
            value_ += ",";
        }
        value_ += text;
    }
    return *this;
}

// IDA_EA: 0x8376034
// IDA_NAME: _ZN11sync_script12CStringMaker5c_strEv
// DEMANGLED: sync_script::CStringMaker::c_str(void)
const char* CStringMaker::c_str() { return value_.c_str(); }

// IDA_EA: 0x861CF0A
// IDA_NAME: _ZN11sync_script11CSyncScript7GetTypeESs17ENUM_CHARACTERJOBiRiS2_b
// DEMANGLED: sync_script::CSyncScript::GetType(std::string,ENUM_CHARACTERJOB,int,int &,int &,bool)
int CSyncScript::GetType(std::string type_name,
                         ENUM_CHARACTERJOB character_job,
                         int equipment_type,
                         int& master_type,
                         int& sub_no,
                         bool equipment) {
    if (type_name.empty() || type_name.find_first_of('[', 0) != 0) {
        return 0;
    }
    type_name.erase(type_name.begin());
    type_name.erase(type_name.end() - 1);

    std::string lookup("stackable ");
    if (!equipment && type_name.compare("creature") != 0) {
        lookup += type_name;
    } else {
        lookup = type_name;
    }

    SITEMTYPE item(item_types_[lookup]);
    if (!type_additions_.empty() && item.master_type == type_additions_[0]) {
        SWEAPONTYPE key(character_job, equipment_type);
        item = weapon_types_[key];
    } else if (type_additions_.size() > 1 &&
               item.master_type == type_additions_[1]) {
        SARMORTYPE key(static_cast<unsigned char>(equipment_type), lookup);
        item = armor_types_[key];
    }
    master_type = item.master_type;
    sub_no = item.sub_no;
    return 1;
}

// IDA_EA: 0x861D2DC
// IDA_NAME: _ZN11sync_script11CSyncScript10GetUserJobE17ENUM_CHARACTERJOB
// DEMANGLED: sync_script::CSyncScript::GetUserJob(ENUM_CHARACTERJOB)
const char* CSyncScript::GetUserJob(ENUM_CHARACTERJOB character_job) {
    switch (character_job) {
        case oldsrv::config::kCharacterJobSwordman:
            return "\xB1\xCD\xB0\xCB\xBB\xE7";
        case oldsrv::config::kCharacterJobFighter:
            return "\xB0\xDD\xC5\xF5\xB0\xA1";
        case oldsrv::config::kCharacterJobGunner:
            return "\xB0\xC5\xB3\xCA";
        case oldsrv::config::kCharacterJobMage:
            return "\xB8\xB6\xB9\xFD\xBB\xE7";
        case oldsrv::config::kCharacterJobPriest:
            return "\xC7\xC1\xB8\xAE\xBD\xBA\xC6\xAE";
        case oldsrv::config::kCharacterJobAtGunner:
            return "\xBF\xA9\xB0\xC5\xB3\xCA";
        case oldsrv::config::kCharacterJobThief:
            return "\xB5\xB5\xC0\xFB";
        case oldsrv::config::kCharacterJobAtFighter:
            return "\xB3\xB2\xB0\xDD\xC5\xF5\xB0\xA1";
        default: return "";
    }
}

// IDA_EA: 0x861D330
// IDA_NAME: _ZN11sync_script11CSyncScript18GetUsableCharacterEP17STEquipmentScript
// DEMANGLED: sync_script::CSyncScript::GetUsableCharacter(STEquipmentScript *)
const char* CSyncScript::GetUsableCharacter(STEquipmentScript* script) {
    if (script->item.value_a0) {
        return "\xB8\xF0\xB5\xCE";
    }
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        if (script->item.usable_job_flags[job]) {
            return GetUserJob(static_cast<ENUM_CHARACTERJOB>(job));
        }
    }
    return "";
}

// IDA_EA: 0x861D38E
// IDA_NAME: _ZN11sync_script11CSyncScript18GetUsableCharacterEP17STStackableScript
// DEMANGLED: sync_script::CSyncScript::GetUsableCharacter(STStackableScript *)
const char* CSyncScript::GetUsableCharacter(STStackableScript* script) {
    if (script->item.value_a0) {
        return "\xB8\xF0\xB5\xCE";
    }
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        if (script->item.usable_job_flags[job]) {
            return GetUserJob(static_cast<ENUM_CHARACTERJOB>(job));
        }
    }
    return "";
}

// IDA_EA: 0x861D3EC
// IDA_NAME: _ZN11sync_script11CSyncScript13GetAttachTypeEi
// DEMANGLED: sync_script::CSyncScript::GetAttachType(int)
const char* CSyncScript::GetAttachType(int attach_type) {
    switch (attach_type) {
        case 0: return "\xC0\xDA\xC0\xAF";
        case 1: return "\xB1\xB3\xC8\xAF";
        case 2: return "\xB1\xB3\xC8\xAF\xBB\xE8\xC1\xA6";
        case 3: return "\xB9\xD0\xBA\xC0";
        case 5: return "\xB0\xE8\xC1\xA4";
        default: return "";
    }
}

// IDA_EA: 0x861D42E
// IDA_NAME: _ZN11sync_script11CSyncScript16GetAttackElementEP22EquipmentParameterInfo
// DEMANGLED: sync_script::CSyncScript::GetAttackElement(EquipmentParameterInfo *)
const char* CSyncScript::GetAttackElement(EquipmentParameterInfo* info) {
    switch (info->values_140[0].integer) {
        case 0: return "\xC8\xAD";
        case 1: return "\xBC\xF6";
        case 2: return "\xBE\xCF";
        case 3: return "\xB8\xED";
        case 4: return "\xB9\xAB";
        default: return "";
    }
}

// IDA_EA: 0x861D472
// IDA_NAME: _ZN11sync_script11CSyncScript15GetLevelUpSkillEP17STEquipmentScriptN22EquipmentParameterInfo26ENUM_SKILL_UP_VECTOR_INDEXE
// DEMANGLED: sync_script::CSyncScript::GetLevelUpSkill(STEquipmentScript *,EquipmentParameterInfo::ENUM_SKILL_UP_VECTOR_INDEX)
const char* CSyncScript::GetLevelUpSkill(
    STEquipmentScript* script,
    EquipmentParameterInfo::ENUM_SKILL_UP_VECTOR_INDEX index) {
    static std::string level_up_skill;
    level_up_skill.clear();
    std::vector<LevelUpSkill>* skills =
        index == EquipmentParameterInfo::SKILL_UP_VECTOR_INDEX_0
            ? &script->parameter_1c0.skill_levelups_24c
            : &script->parameter_1c0.skill_levelups_258;
    for (std::vector<LevelUpSkill>::iterator it = skills->begin();
         it != skills->end(); ++it) {
        level_up_skill += "|";
        const char* job =
            GetUserJob(static_cast<ENUM_CHARACTERJOB>(it->skill_type));
        if (*job == '\0') {
            level_up_skill += "\xB0\xF8\xC5\xEB";
        } else {
            level_up_skill += job;
        }
        char text[1024];
        std::memset(text, 0, sizeof(text));
        std::sprintf(text, "| %d| %d", it->skill_id, it->skill_level);
        level_up_skill += text;
    }
    return level_up_skill.c_str();
}

// IDA_EA: 0x861D706
// IDA_NAME: _ZN11sync_script11CSyncScript6GetURLEPKcii
// DEMANGLED: sync_script::CSyncScript::GetURL(char const*,int,int)
const char* CSyncScript::GetURL(const char* filename,
                                int filename_length,
                                int icon_id) {
    static std::string url;
    url.clear();
    int name_start = 0;
    int extension_start = 0;
    for (int index = 0; index < filename_length; ++index) {
        if (filename[index] == '/') {
            name_start = index + 1;
        }
        if (filename[index] == '.') {
            extension_start = index;
        }
    }
    url.insert(0, &filename[name_start], extension_start - name_start);
    char suffix[16];
    std::memset(suffix, 0, sizeof(suffix));
    std::sprintf(suffix, "/%05d.png", icon_id);
    url += suffix;
    return url.c_str();
}

// IDA_EA: 0x8622374
// IDA_NAME: _ZN11sync_script11CSyncScript28truncate_dnf_item_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_dnf_item_info_table(void)
bool CSyncScript::truncate_dnf_item_info_table() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table dnf_item_info");
    if (db_handle->exec(true) != true) {
        return false;
    }
    db_handle->set_query("trUncate table avatar_select_ability");
    return db_handle->exec(true) == true &&
           truncate_artifact_info_table() == true;
}

// IDA_EA: 0x86225B6
// IDA_NAME: _ZN11sync_script11CSyncScript23truncate_quest_categoryEv
// DEMANGLED: sync_script::CSyncScript::truncate_quest_category(void)
bool CSyncScript::truncate_quest_category() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table quest_category");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x86227F0
// IDA_NAME: _ZN11sync_script11CSyncScript20truncate_skill_indexEv
// DEMANGLED: sync_script::CSyncScript::truncate_skill_index(void)
bool CSyncScript::truncate_skill_index() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table skill_index");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622866
// IDA_NAME: _ZN11sync_script11CSyncScript28truncate_exp_level_ref_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_exp_level_ref_table(void)
bool CSyncScript::truncate_exp_level_ref_table() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table exp_level_ref");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x86228DC
// IDA_NAME: _ZN11sync_script11CSyncScript22insert_exp_level_to_dbEji
// DEMANGLED: sync_script::CSyncScript::insert_exp_level_to_db(unsigned int,int)
bool CSyncScript::insert_exp_level_to_db(unsigned int exp, int level) {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("inSert into exp_level_ref(exp, lev) values(%u, %d)",
                         exp, level);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622960
// IDA_NAME: _ZN11sync_script11CSyncScript25truncate_charac_quest_refEv
// DEMANGLED: sync_script::CSyncScript::truncate_charac_quest_ref(void)
bool CSyncScript::truncate_charac_quest_ref() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table charac_quest_ref; ");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x86229D6
// IDA_NAME: _ZN11sync_script11CSyncScript28insert_charac_quest_categoryEjj
// DEMANGLED: sync_script::CSyncScript::insert_charac_quest_category(unsigned int,unsigned int)
bool CSyncScript::insert_charac_quest_category(unsigned int origin_index,
                                               unsigned int mapped_index) {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into charac_quest_ref(origin_idx, mapped_idx) values(%d, %d)",
        origin_index, mapped_index);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622A5A
// IDA_NAME: _ZN11sync_script11CSyncScript25truncateCeraShopItemTableEv
// DEMANGLED: sync_script::CSyncScript::truncateCeraShopItemTable(void)
int CSyncScript::truncateCeraShopItemTable() {
    if (enable_ != 1) {
        return 1;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table dnf_item_price; ");
    return db_handle->exec(true);
}

// IDA_EA: 0x8622ABC
// IDA_NAME: _ZN11sync_script11CSyncScript24insertCeraShopItemIntoDBEjjiisPKcS2_
// DEMANGLED: sync_script::CSyncScript::insertCeraShopItemIntoDB(unsigned int,unsigned int,int,int,short,char const*,char const*)
int CSyncScript::insertCeraShopItemIntoDB(unsigned int ipg_no,
                                          unsigned int item_no,
                                          int item_count,
                                          int cera_price,
                                          short shop_type,
                                          const char* item_name,
                                          const char* item_comment) {
    if (enable_ != 1 || ipg_no == static_cast<unsigned int>(-1)) {
        return 1;
    }
    char shop_type_text[255];
    std::memset(shop_type_text, 0, sizeof(shop_type_text));
    switch (shop_type) {
        case 1: std::memcpy(shop_type_text, "7\xC0\xCF", 4); break;
        case 2: std::memcpy(shop_type_text, "30\xC0\xCF", 5); break;
        case 3: std::memcpy(shop_type_text, "\xBF\xB5\xB1\xB8", 5); break;
        case 4: std::memcpy(shop_type_text, "\xBF\xA5\xBA\xED\xB7\xBD", 7); break;
        default: break;
    }
    (void)shop_type_text;
    (void)item_name;
    (void)item_comment;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into dnf_item_price(ipg_no, it_no, it_cnt, cera_price) values(%d,%d,%d,%d)",
        ipg_no, item_no, item_count, cera_price);
    return db_handle->exec(true);
}

// IDA_EA: 0x8625064
// IDA_NAME: _ZN11sync_script11CSyncScript24LoadItemInfoMasterScriptEPKc
// DEMANGLED: sync_script::CSyncScript::LoadItemInfoMasterScript(char const*)
int CSyncScript::LoadItemInfoMasterScript(const char* filename) {
    if (loadRDARScriptFile("", filename) != 1) {
        return 0;
    }

    std::string tag;
    std::string text;
    std::string description;
    while (ScanType(tag, true) == 1) {
        if (tag == "[master type]") {
            const int value = ScanInt(static_cast<bool*>(0));
            ScanStr(&text);
            master_type_names_[static_cast<unsigned char>(value)] = text;
        } else if (tag == "[type addition]") {
            const int value = ScanInt(static_cast<bool*>(0));
            const unsigned char item_type =
                static_cast<unsigned char>(value);
            type_additions_.push_back(item_type);
        } else if (tag == "[reverse desc]") {
            const int value = ScanInt(static_cast<bool*>(0));
            const unsigned char item_type =
                static_cast<unsigned char>(value);
            reverse_desc_.push_back(item_type);
        } else if (tag == "[material type]") {
            const int value = ScanInt(static_cast<bool*>(0));
            ScanStr(&text);
            material_type_names_[static_cast<unsigned char>(value)] = text;
        } else if (tag == "[weapon type]") {
            ScanStr(&text);
            const int master_type = ScanInt(static_cast<bool*>(0));
            const int sub_type = ScanInt(static_cast<bool*>(0));
            const int sub_no = ScanInt(static_cast<bool*>(0));
            ScanStr(&description);

            const int character_job = ::getCharacterJob(text.c_str());
            const SITEMTYPE item(false,
                                 static_cast<unsigned char>(master_type),
                                 static_cast<unsigned char>(sub_type),
                                 static_cast<unsigned short>(sub_no),
                                 description);
            const SWEAPONTYPE key(
                static_cast<ENUM_CHARACTERJOB>(character_job), sub_type);
            weapon_types_[key] = item;
        } else if (tag == "[armor type]") {
            ScanStr(&text);
            const int master_type = ScanInt(static_cast<bool*>(0));
            const int sub_type = ScanInt(static_cast<bool*>(0));
            const int sub_no = ScanInt(static_cast<bool*>(0));
            ScanStr(&description);

            const SITEMTYPE item(false,
                                 static_cast<unsigned char>(master_type),
                                 static_cast<unsigned char>(sub_type),
                                 static_cast<unsigned short>(sub_no),
                                 description);
            const SARMORTYPE key(static_cast<unsigned char>(sub_type), text);
            armor_types_[key] = item;
        } else if (tag == "[item type]") {
            ScanStr(&text);
            const int item_type = ScanInt(static_cast<bool*>(0));
            const int master_type = ScanInt(static_cast<bool*>(0));
            const int sub_type = ScanInt(static_cast<bool*>(0));
            const int sub_no = ScanInt(static_cast<bool*>(0));
            ScanStr(&description);

            const SITEMTYPE item(item_type != 0,
                                 static_cast<unsigned char>(master_type),
                                 static_cast<unsigned char>(sub_type),
                                 static_cast<unsigned short>(sub_no),
                                 description);
            item_types_[text] = item;
        }
    }

    return 1;
}

// IDA_EA: 0x8625696
// IDA_NAME: _ZN11sync_script11CSyncScript31truncate_item_info_master_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_item_info_master_table(void)
bool CSyncScript::truncate_item_info_master_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_item_info_master") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x862571A
// IDA_NAME: _ZN11sync_script11CSyncScript29insert_item_info_master_to_dbEv
// DEMANGLED: sync_script::CSyncScript::insert_item_info_master_to_db(void)
int CSyncScript::insert_item_info_master_to_db() {
    if (enable_ != 1) {
        return 1;
    }

    char master_explain[100];
    char item_name[100];
    std::memset(item_name, 0, sizeof(item_name));
    std::memset(master_explain, 0, sizeof(master_explain));

    for (std::map<SWEAPONTYPE, SITEMTYPE>::iterator it =
             weapon_types_.begin();
         it != weapon_types_.end();
         ++it) {
        const SITEMTYPE& item = it->second;
        const char* description = item.description.c_str();
        const char* master_name = master_type_names_[item.master_type].c_str();
        std::sprintf(master_explain, "<%s>%s", master_name, description);
        if (insert_one_item_info_master(item.master_type,
                                        item.sub_no,
                                        description,
                                        master_explain) != true) {
            return 0;
        }
    }

    for (std::map<SARMORTYPE, SITEMTYPE>::iterator it = armor_types_.begin();
         it != armor_types_.end();
         ++it) {
        const SITEMTYPE& item = it->second;
        const char* description = item.description.c_str();
        const char* material_name =
            material_type_names_[item.sub_type].c_str();
        std::sprintf(item_name, "%s%s", material_name, description);
        const char* master_name = master_type_names_[item.master_type].c_str();
        std::sprintf(master_explain, "<%s>%s", master_name, item_name);
        if (insert_one_item_info_master(item.master_type,
                                        item.sub_no,
                                        item_name,
                                        master_explain) != true) {
            return 0;
        }
    }

    for (std::map<std::string, SITEMTYPE>::iterator it = item_types_.begin();
         it != item_types_.end();
         ++it) {
        const SITEMTYPE& item = it->second;
        if (item.item_type != 1) {
            continue;
        }

        bool found_addition = false;
        for (std::vector<unsigned char>::iterator addition =
                 type_additions_.begin();
             addition != type_additions_.end();
             ++addition) {
            if (item.master_type == *addition) {
                found_addition = true;
                break;
            }
        }

        const char* description = item.description.c_str();
        const char* master_name = master_type_names_[item.master_type].c_str();
        std::sprintf(master_explain, "<%s>%s", master_name, description);
        if (found_addition) {
            if (insert_one_item_info_master(item.master_type,
                                            item.sub_no,
                                            master_name,
                                            master_explain) != true) {
                return 0;
            }
        } else {
            if (insert_one_item_info_master(item.master_type,
                                            item.sub_no,
                                            description,
                                            master_explain) != true) {
                return 0;
            }
        }
    }

    return 1;
}

// IDA_EA: 0x8625C54
// IDA_NAME: _ZN11sync_script11CSyncScript27insert_one_item_info_masterEhtPKcS2_
// DEMANGLED: sync_script::CSyncScript::insert_one_item_info_master(unsigned char,unsigned short,char const*,char const*)
bool CSyncScript::insert_one_item_info_master(unsigned char master_no,
                                              unsigned short sub_no,
                                              const char* name,
                                              const char* master_explain) {
    char escaped[200];
    const char* strings[2];
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));

    strings[0] = ::toMbcs(name);
    strings[1] = ::toMbcs(master_explain);
    std::memset(escaped, 0, sizeof(escaped));
    for (unsigned int index = 0; index <= 1; ++index) {
        ::GetEscapeString(db_handle, strings[index], &escaped[100 * index]);
    }

    return db_handle->set_query(
               "inSert into dnf_item_info_master(master_no, sub_no, name, master_explain) values(%u, %u, '%s', '%s')",
               master_no,
               sub_no,
               escaped,
               &escaped[100]) == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8622C66
// IDA_NAME: _ZN11sync_script11CSyncScript23trancate_item_parts_setEv
// DEMANGLED: sync_script::CSyncScript::trancate_item_parts_set(void)
bool CSyncScript::trancate_item_parts_set() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table item_part_set");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622CDC
// IDA_NAME: _ZN11sync_script11CSyncScript21insert_item_parts_setEiR23STEquipmentPartSetPiece
// DEMANGLED: sync_script::CSyncScript::insert_item_parts_set(int,STEquipmentPartSetPiece &)
bool CSyncScript::insert_item_parts_set(
    int part_set_index,
    ::STEquipmentPartSetPiece& piece) {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into item_part_set(part_set_index, part_name, part_type, part_grade, part_rarity) values(%d, '%s',%d,%d,%d)",
        part_set_index,
        piece.part_name.c_str(),
        piece.part_type,
        piece.part_grade,
        piece.part_rarity);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8624390
// IDA_NAME: _ZN11sync_script11CSyncScript28truncate_creature_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_creature_info_table(void)
bool CSyncScript::truncate_creature_info_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_creature_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8624414
// IDA_NAME: _ZN11sync_script11CSyncScript26insert_creature_info_to_dbEjPK14CreatureScriptPKcS5_
// DEMANGLED: sync_script::CSyncScript::insert_creature_info_to_db(unsigned int,CreatureScript const*,char const*,char const*)
int CSyncScript::insert_creature_info_to_db(unsigned int item_id,
                                            const ::CreatureScript* script,
                                            const char* skill_info,
                                            const char* overskill_info) {
    if (enable_ != 1) {
        return 1;
    }

    char artifact_slot[4] = {0, 0, 0, 0};
    for (unsigned int index = 0; index <= 2; ++index) {
        artifact_slot[index] = static_cast<char>(script->artifact_slot[index] +
                                                 '0');
    }

    CStringMaker piercing;
    for (std::vector<int>::const_iterator it = script->piercing.begin();
         it != script->piercing.end();
         ++it) {
        piercing(*it);
    }

    CStringMaker skill_level_values;
    for (std::vector<int>::const_iterator it = script->skill_level_values.begin();
         it != script->skill_level_values.end();
         ++it) {
        skill_level_values(*it);
    }

    CStringMaker over_skill_level_values;
    for (std::vector<int>::const_iterator it =
             script->over_skill_level_values.begin();
         it != script->over_skill_level_values.end();
         ++it) {
        over_skill_level_values(*it);
    }

    const char* values[11];
    values[0] = script->name.c_str();
    values[1] = artifact_slot;
    values[2] = skill_info;
    values[3] = overskill_info;
    values[4] = piercing.c_str();
    values[5] = script->skill_strings.size() > 0
                    ? script->skill_strings[0].c_str()
                    : "";
    values[6] = script->skill_strings.size() > 1
                    ? script->skill_strings[1].c_str()
                    : "";
    values[7] = script->skill_strings.size() > 2
                    ? script->skill_strings[2].c_str()
                    : "";
    values[8] = script->skill_strings.size() > 3
                    ? script->skill_strings[3].c_str()
                    : "";
    values[9] = skill_level_values.c_str();
    values[10] = over_skill_level_values.c_str();

    char escaped[2816];
    std::memset(escaped, 0, sizeof(escaped));
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    for (unsigned int index = 0; index <= 10; ++index) {
        ::GetEscapeString(db_handle, values[index], &escaped[256 * index]);
    }

    return db_handle->set_query(
               "inSert into dnf_creature_info(it_id, creature_id, creature_name, skill_recovery_time, overskill_recovery_time, artifact_slot, learn_overskill_level, skill_info, overskill_info, piercing, skill_name, skill_desc, overskill_name, overskill_desc, skill_level_values, overskill_level_values, evolution_creature_id, evolution_level) values(%u, %d, '%s', %d, %d, '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d)",
               item_id,
               script->index,
               escaped,
               script->skill_recovery_time,
               script->over_skill_recovery_time,
               &escaped[256],
               script->learn_overskill_level,
               &escaped[512],
               &escaped[768],
               &escaped[1024],
               &escaped[1280],
               &escaped[1536],
               &escaped[1792],
               &escaped[2048],
               &escaped[2304],
               &escaped[2560],
               script->evolution_creature_id,
               script->evolution_level) == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8624B0A
// IDA_NAME: _ZN11sync_script11CSyncScript28truncate_artifact_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_artifact_info_table(void)
bool CSyncScript::truncate_artifact_info_table() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_artifact_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8624B8E
// IDA_NAME: _ZN11sync_script11CSyncScript26insert_artifact_info_to_dbEP17STEquipmentScript
// DEMANGLED: sync_script::CSyncScript::insert_artifact_info_to_db(STEquipmentScript *)
bool CSyncScript::insert_artifact_info_to_db(STEquipmentScript* script) {
    if (enable_ != 1) {
        return true;
    }
    const STEquipmentScript::artifact_info& artifact = script->artifact_738;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query(
               "inSert into dnf_artifact_info(it_id, creature_min_level, physical_attack, magical_attack, skill_consume_mp_rate, skill_charge_time_rate, skill_overcharge_time_rate, experience_amount, physical_critical_hit, magical_critical_hit, stuck) values(%d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f)",
               script->item.index,
               artifact.usable_type,
               artifact.grade,
               artifact.rarity,
               artifact.physical_attack,
               artifact.magical_attack,
               artifact.physical_defense,
               artifact.magical_defense,
               artifact.strength,
               artifact.vitality,
               artifact.spirit) == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8624C96
// IDA_NAME: _ZN11sync_script11CSyncScript29truncate_old_equip_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_old_equip_info_table(void)
bool CSyncScript::truncate_old_equip_info_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_old_equip_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8624D1A
// IDA_NAME: _ZN11sync_script11CSyncScript27insert_old_equip_info_to_dbEv
// DEMANGLED: sync_script::CSyncScript::insert_old_equip_info_to_db(void)
int CSyncScript::insert_old_equip_info_to_db() {
    if (enable_ != 1) {
        return 1;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    for (std::map<unsigned int, ::STEquipmentStatInfo>::iterator it =
             ::g_equipmentStatInfoMap.begin();
         it != ::g_equipmentStatInfoMap.end();
         ++it) {
        const ::STEquipmentStatInfo& stat_info = it->second;
        if (db_handle->set_query(
                "inSert into dnf_old_equip_info(it_id, hp_max, mp_max, phy_att, phy_def, mag_att, mag_def, equip_phy_att, equip_phy_def, equip_mag_att, equip_mag_def, ref_fire, ref_water, ref_dark, ref_light, ref_all_elements, ref_slow, ref_freeze, ref_poison, ref_stun, ref_curse, ref_blind, ref_lightning, ref_stone, ref_sleep, ref_burn, ref_weapon_break, ref_bleeding, ref_pierce, ref_stuck, ref_confuse, ref_hold, ref_armor_break, ref_all_state) values(%d, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)",
                static_cast<int>(it->first),
                stat_info.hp_max,
                stat_info.mp_max,
                stat_info.physical_attack,
                stat_info.physical_defense,
                stat_info.magical_attack,
                stat_info.magical_defense,
                stat_info.equipment_physical_attack,
                stat_info.equipment_physical_defense,
                stat_info.equipment_magical_attack,
                stat_info.equipment_magical_defense,
                stat_info.element_tolerance[0],
                stat_info.element_tolerance[1],
                stat_info.element_tolerance[2],
                stat_info.element_tolerance[3],
                stat_info.all_element_tolerance,
                stat_info.active_property[0],
                stat_info.active_property[1],
                stat_info.active_property[2],
                stat_info.active_property[3],
                stat_info.active_property[4],
                stat_info.active_property[5],
                stat_info.active_property[6],
                stat_info.active_property[7],
                stat_info.active_property[8],
                stat_info.active_property[9],
                stat_info.active_property[10],
                stat_info.active_property[11],
                stat_info.active_property[12],
                stat_info.active_property[13],
                stat_info.active_property[14],
                stat_info.active_property[15],
                stat_info.active_property[16],
                stat_info.all_active_status_tolerance) != true) {
            return 0;
        }
        if (db_handle->exec(true) != true) {
            return 0;
        }
    }

    return 1;
}

// IDA_EA: 0x8625D96
// IDA_NAME: _ZN11sync_script11CSyncScript33truncate_charac_action_point_descEv
// DEMANGLED: sync_script::CSyncScript::truncate_charac_action_point_desc(void)
bool CSyncScript::truncate_charac_action_point_desc() {
    return enable_ != 1 || ::APSystem::CSyncScript::TruncateDescTable();
}

// IDA_EA: 0x8625DB8
// IDA_NAME: _ZN11sync_script11CSyncScript31insert_charac_action_point_descEv
// DEMANGLED: sync_script::CSyncScript::insert_charac_action_point_desc(void)
bool CSyncScript::insert_charac_action_point_desc() {
    return enable_ != 1 || ::APSystem::CSyncScript::InsertDescTable();
}

// IDA_EA: 0x8625DDA
// IDA_NAME: _ZN11sync_script11CSyncScript39truncate_charac_advance_altar_item_descEv
// DEMANGLED: sync_script::CSyncScript::truncate_charac_advance_altar_item_desc(void)
bool CSyncScript::truncate_charac_advance_altar_item_desc() {
    return enable_ != 1 ||
           ::advancealtar::SyncScript::truncateItemDescTable();
}

// IDA_EA: 0x8625DFC
// IDA_NAME: _ZN11sync_script11CSyncScript37insert_charac_advance_altar_item_descEv
// DEMANGLED: sync_script::CSyncScript::insert_charac_advance_altar_item_desc(void)
bool CSyncScript::insert_charac_advance_altar_item_desc() {
    return enable_ != 1 ||
           ::advancealtar::SyncScript::insertItemDescTable();
}

// IDA_EA: 0x8625E1E
// IDA_NAME: _ZN11sync_script11CSyncScript26truncate_random_option_refEv
// DEMANGLED: sync_script::CSyncScript::truncate_random_option_ref(void)
bool CSyncScript::truncate_random_option_ref() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table random_option_ref") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8625EA2
// IDA_NAME: _ZN11sync_script11CSyncScript24insert_random_option_refEv
// DEMANGLED: sync_script::CSyncScript::insert_random_option_ref(void)
int CSyncScript::insert_random_option_ref() {
    if (enable_ != 1) {
        return 1;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    std::string prefix;
    for (unsigned char random_option_index = 1; random_option_index != 0xFFu;
         ++random_option_index) {
        for (unsigned char random_option_value = 0;
             random_option_value <= 0x64u;
             ++random_option_value) {
            RandomOptionScript* random_option_script =
                G_CDataManager()->get_random_option_script();
            prefix = random_option_script->getPrefix(random_option_index,
                                                     random_option_value,
                                                     false);
            if (prefix.length() == 0) {
                continue;
            }

            char converted[1024];
            std::memset(converted, 0, sizeof(converted));
            char* source = const_cast<char*>(prefix.c_str());
            if (CodePage::script2Database(source, converted) != 1) {
                CodePage::database();
                CodePage::script();
                cMyTrace trace(
                    "bool sync_script::CSyncScript::insert_random_option_ref()",
                    2865,
                    5);
                trace("Error!!! %s->%s [%s][%s][%d]",
                      CodePage::script(),
                      CodePage::database(),
                      source,
                      converted,
                      random_option_index);
            }
            db_handle->set_query(
                "inSert into random_option_ref(random_option_index, random_option_value, random_option_name) values(%d,%d, '%s')",
                random_option_index,
                random_option_value,
                converted);
            if (db_handle->exec(true) != true) {
                return 0;
            }
        }
    }

    return 1;
}

// IDA_EA: 0x862676C
// IDA_NAME: _ZN11sync_script11CSyncScript33truncate_mapping_equip_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_mapping_equip_info_table(void)
bool CSyncScript::truncate_mapping_equip_info_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table equip_mapping_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x86267F0
// IDA_NAME: _ZN11sync_script11CSyncScript31insert_mapping_equip_info_to_dbERSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: sync_script::CSyncScript::insert_mapping_equip_info_to_db(std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> &)
int CSyncScript::insert_mapping_equip_info_to_db(std::map<int, int>& mapping) {
    if (enable_ != 1) {
        return 1;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    for (std::map<int, int>::iterator it = mapping.begin();
         it != mapping.end();
         ++it) {
        if (it->second != 0) {
            db_handle->set_query(
                "inSert into equip_mapping_info(equip_idx, mapping_idx) values(%d, %d)",
                it->second,
                it->first);
            if (db_handle->exec(true) != true) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x86260DC
// IDA_NAME: _ZN11sync_script11CSyncScript27truncate_monster_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_monster_info_table(void)
bool CSyncScript::truncate_monster_info_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_monster_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8626160
// IDA_NAME: _ZN11sync_script11CSyncScript25insert_monster_info_to_dbERK15STMonsterScript
// DEMANGLED: sync_script::CSyncScript::insert_monster_info_to_db(STMonsterScript const&)
int CSyncScript::insert_monster_info_to_db(const ::STMonsterScript& script) {
    if (enable_ != 1) {
        return 1;
    }

    const int monster_index = MonsterScriptIndex(script);
    const std::string& monster_name = MonsterScriptName(script);
    if (monster_index == 0 || monster_name.length() == 0) {
        return 1;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    char database_name[1024];
    char escaped_name[2048];
    std::memset(database_name, 0, sizeof(database_name));
    std::memset(escaped_name, 0, sizeof(escaped_name));

    if (CodePage::script2Database(
            const_cast<char*>(monster_name.c_str()),
            database_name) != 1) {
        return 0;
    }

    db_handle->escape_string(escaped_name, database_name);
    db_handle->set_query(
        "inSert into dnf_monster_info(idx, mon_name_kr) values(%d, '%s')",
        monster_index,
        escaped_name);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x862630E
// IDA_NAME: _ZN11sync_script11CSyncScript31truncate_aicharacter_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_aicharacter_info_table(void)
bool CSyncScript::truncate_aicharacter_info_table() {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_aicharacter_info") ==
               true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8626392
// IDA_NAME: _ZN11sync_script11CSyncScript29insert_aicharacter_info_to_dbEiRK25STAICharacterCommonScript
// DEMANGLED: sync_script::CSyncScript::insert_aicharacter_info_to_db(int,STAICharacterCommonScript const&)
int CSyncScript::insert_aicharacter_info_to_db(
    int index,
    const ::STAICharacterCommonScript& script) {
    if (enable_ != 1) {
        return 1;
    }

    const std::string& ai_name = AICharacterScriptName(script);
    if (index == 0 || ai_name.length() == 0) {
        return 1;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    char database_name[1024];
    char escaped_name[2048];
    std::memset(database_name, 0, sizeof(database_name));
    std::memset(escaped_name, 0, sizeof(escaped_name));

    if (CodePage::script2Database(const_cast<char*>(ai_name.c_str()),
                                  database_name) != 1) {
        return 0;
    }

    db_handle->escape_string(escaped_name, database_name);
    db_handle->set_query(
        "inSert into dnf_aicharacter_info(idx, ai_name_kr) values(%d, '%s')",
        index,
        escaped_name);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x862653A
// IDA_NAME: _ZN11sync_script11CSyncScript24truncate_town_info_tableEv
// DEMANGLED: sync_script::CSyncScript::truncate_town_info_table(void)
bool CSyncScript::truncate_town_info_table() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    return db_handle->set_query("trUncate table dnf_town_info") == true &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x86265BE
// IDA_NAME: _ZN11sync_script11CSyncScript22insert_town_info_to_dbERK10TownScript
// DEMANGLED: sync_script::CSyncScript::insert_town_info_to_db(TownScript const&)
bool CSyncScript::insert_town_info_to_db(const TownScript& script) {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    if (script.town_index == 0 || script.town_name.length() == 0) {
        return true;
    }
    static char town_name[240];
    char converted_name[1024];
    std::memset(town_name, 0, sizeof(town_name));
    std::memset(converted_name, 0, sizeof(converted_name));
    if (CodePage::script2Database(
            const_cast<char*>(script.town_name.c_str()), converted_name) != 1) {
        CodePage::database();
        CodePage::script();
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_town_info_to_db(const TownScript&)",
            3010,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script.town_name.c_str(), converted_name,
              script.town_index);
    }
    db_handle->escape_string(town_name, converted_name);
    db_handle->set_query(
        "inSert into dnf_town_info(idx, town_name_kr) values(%d, '%s')",
        script.town_index,
        town_name);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8620968
// IDA_NAME: _ZN11sync_script11CSyncScript27insert_stackable_item_to_dbEP17STStackableScript
// DEMANGLED: sync_script::CSyncScript::insert_stackable_item_to_db(STStackableScript *)
bool CSyncScript::insert_stackable_item_to_db(STStackableScript* script) {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    int master_type;
    int sub_type;
    if (GetType(script->item.strings_ac[4],
                static_cast<ENUM_CHARACTERJOB>(
                    oldsrv::config::kCharacterJobSwordman),
                0, master_type, sub_type,
                false) != 1) {
        return false;
    }

    char converted_name[1024];
    char converted_explain[1024];
    char escaped[4][4096];
    std::memset(converted_name, 0, sizeof(converted_name));
    std::memset(converted_explain, 0, sizeof(converted_explain));
    std::memset(escaped, 0, sizeof(escaped));

    if (CodePage::script2Database(
            const_cast<char*>(script->item.strings_ac[0].c_str()),
            converted_name) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_stackable_item_to_db(STStackableScript*)",
            1315,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->item.strings_ac[0].c_str(),
              converted_name, script->item.index);
        return false;
    }
    if (CodePage::script2Database(
            const_cast<char*>(script->item.strings_ac[2].c_str()),
            converted_explain) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_stackable_item_to_db(STStackableScript*)",
            1322,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->item.strings_ac[2].c_str(),
              converted_explain, script->item.index);
        return false;
    }

    ::GetEscapeString(db_handle, converted_name, escaped[0]);
    ::GetEscapeString(db_handle, script->item.strings_ac[1].c_str(),
                      escaped[1]);
    ::GetEscapeString(db_handle, converted_explain, escaped[2]);

    CStreamBuffer values;
    values << script->item.index;
    AppendSqlText(values, escaped[0]);
    AppendSqlText(values, escaped[1]);
    AppendSqlText(values, escaped[2]);
    AppendSqlInteger(values, master_type);
    AppendSqlInteger(values, sub_type);
    AppendSqlText(values, GetUsableCharacter(script));
    AppendSqlInteger(values, script->item.grade);
    AppendSqlText(values, GetAttachType(script->item.values_08[0]));
    AppendSqlInteger(values, script->item.values_5c[0]);
    AppendSqlInteger(values, 0);
    AppendSqlInteger(values, script->item.values_08[1]);
    AppendSqlInteger(values, script->item.values_08[2]);
    AppendSqlInteger(values, script->item.value_34);
    AppendSqlInteger(values, script->item.values_08[3]);
    AppendSqlInteger(values, script->item.values_08[4]);
    AppendSqlInteger(values, script->item.values_08[5]);
    AppendSqlInteger(values, 0);
    AppendSqlInteger(values, script->item.values_5c[3]);

    // The original uses map::operator[] with module key 19 and therefore
    // preserves its null-module branch as real database values, not fake data.
    AppendStackableModuleParameter(values, this,
                                   script->module_parameters_4bc[19]);
    AppendSqlText(values, "");

    const char* url = GetURL(script->item.icon.c_str(),
                             script->item.icon.length(),
                             script->item.icon_index);
    ::GetEscapeString(db_handle, url, escaped[3]);
    AppendSqlText(values, escaped[3]);
    AppendSqlInteger(values, script->item.values_c4[0]);

    db_handle->set_query(
        "inSert into dnf_item_info(it_no, it_name, it_eng_name, it_explain, master_type, sub_type, job, class, revert, level, skill, create_ratio, rarity, weight, price, cash, medal, durability, cooltime, hp_max, mp_max, phy_att, phy_def, mag_att, mag_def, equip_phy_att, equip_phy_def, equip_mag_att, equip_mag_def, ref_fire, ref_water, ref_dark, ref_light, ref_all, ref_slow, ref_freeze, ref_poison, ref_stun, ref_cus, ref_blind, ref_lite, ref_ston, ref_sleep, ref_deekement, ref_deadlystrike, ref_bleeding, ref_confuse, ref_hold, ref_all_stat, ref_pierce, ref_stuck, inven_max, hp_regenrate, mp_regenrate, mov_speed, att_speed, quest, hit_recovery, jump, att_element, att_active_status, att_active_status_ratio, att_active_status_pow, att_backforce, att_upforce, att_hp_drain, att_mp_drain, criticalhit_rate, stuck_rate, att_defenseIgnore, room_list_move_speed_rate, hp_max_rate, mp_max_rate, fire_attack, water_attack, dark_attack, light_attack, all_attack, rigidity, magical_critical_hit, physical_absolute_damage, physical_damage_reduce, physical_absolute_defense, magical_absolute_damage, magical_damage_reduce, magical_absolute_defense, armor_break_resistance, skill_levelup, set_type, url, icon_mark_number) values(%s)",
        values.c_str());
    CFieldLengthChecker checker;
    checker.AddField(db_handle->GetQueryString());
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x862243A
// IDA_NAME: _ZN11sync_script11CSyncScript21insert_quest_categoryEP11QuestScript
// DEMANGLED: sync_script::CSyncScript::insert_quest_category(QuestScript *)
bool CSyncScript::insert_quest_category(QuestScript* script) {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    char converted_name[256];
    char escaped_name[256];
    std::memset(converted_name, 0, sizeof(converted_name));
    std::memset(escaped_name, 0, sizeof(escaped_name));
    if (CodePage::script2Database(const_cast<char*>(script->name.c_str()),
                                  converted_name) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_quest_category(QuestScript*)",
            1700,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->name.c_str(), converted_name,
              script->quest_index);
    }
    db_handle->escape_string(escaped_name, converted_name);
    db_handle->set_query(
        "inSert into quest_category(quest_idx, quest_name) values(%d, '%s')",
        script->quest_index,
        escaped_name);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x862262C
// IDA_NAME: _ZN11sync_script11CSyncScript18insert_skill_indexEP13STSkillScripti
// DEMANGLED: sync_script::CSyncScript::insert_skill_index(STSkillScript *,int)
bool CSyncScript::insert_skill_index(STSkillScript* script, int job) {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char converted_name[255];
    char escaped_name[765];
    std::memset(converted_name, 0, sizeof(converted_name));
    std::memset(escaped_name, 0, sizeof(escaped_name));
    if (CodePage::script2Database(const_cast<char*>(script->name.c_str()),
                                  converted_name) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_skill_index(STSkillScript*, int)",
            1760,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->name.c_str(), converted_name,
              script->skill_index);
    }
    db_handle->escape_string(escaped_name, converted_name);
    db_handle->set_query(
        "inSert into skill_index(job, skill_idx, skill_name) values(%d, %d, '%s')",
        job,
        script->skill_index,
        escaped_name);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622D92
// IDA_NAME: _ZN11sync_script11CSyncScript19trancate_skill_infoEv
// DEMANGLED: sync_script::CSyncScript::trancate_skill_info(void)
bool CSyncScript::trancate_skill_info() {
    if (enable_ != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("trUncate table skill_info; ");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8622E08
// IDA_NAME: _ZN11sync_script11CSyncScript17insert_skill_infoEP13STSkillScriptii
// DEMANGLED: sync_script::CSyncScript::insert_skill_info(STSkillScript *,int,int)
bool CSyncScript::insert_skill_info(STSkillScript* script,
                                    int job,
                                    int module_type) {
    if (enable_ != 1) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    char converted_name[1024];
    char converted_name2[1024];
    char escaped[11][4096];
    std::memset(converted_name, 0, sizeof(converted_name));
    std::memset(converted_name2, 0, sizeof(converted_name2));
    std::memset(escaped, 0, sizeof(escaped));

    if (CodePage::script2Database(const_cast<char*>(script->name.c_str()),
                                  converted_name) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_skill_info(STSkillScript*, int, int)",
            2047,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->name.c_str(), converted_name,
              script->skill_index);
    }
    if (CodePage::script2Database(const_cast<char*>(script->name2.c_str()),
                                  converted_name2) != 1) {
        cMyTrace trace(
            "bool sync_script::CSyncScript::insert_skill_info(STSkillScript*, int, int)",
            2055,
            5);
        trace("Error!!! %s->%s [%s][%s][%d]", CodePage::script(),
              CodePage::database(), script->name2.c_str(), converted_name2,
              script->skill_index);
    }

    ::GetEscapeString(db_handle, converted_name, escaped[0]);
    ::GetEscapeString(db_handle, converted_name2, escaped[1]);
    ::GetEscapeString(db_handle, script->basic_explain.c_str(), escaped[2]);
    ::GetEscapeString(db_handle, script->explain.c_str(), escaped[3]);
    ::GetEscapeString(db_handle, script->icon_name.c_str(), escaped[4]);
    ::GetEscapeString(db_handle, script->icon_sprite.c_str(), escaped[5]);
    ::GetEscapeString(db_handle, script->command_key_explain.c_str(),
                      escaped[6]);

    std::vector<int> pre_required;
    unsigned int index;
    for (index = 0; index < script->pre_required_skill_indices.size();
         ++index) {
        pre_required.push_back(script->pre_required_skill_indices[index]);
        pre_required.push_back(script->pre_required_skill_levels[index]);
    }

    CStringMaker consume_item_maker;
    std::vector<SkillUseItem>::const_iterator consume_it;
    for (consume_it = script->consume_items.begin();
         consume_it != script->consume_items.end();
         ++consume_it) {
        consume_item_maker(*consume_it);
    }

    CStringMaker level_info_maker;
    std::vector<std::vector<float> >::const_iterator level_info_it;
    for (level_info_it = script->cooltime_level_info.begin();
         level_info_it != script->cooltime_level_info.end();
         ++level_info_it) {
        std::vector<float> level_values(*level_info_it);
        level_info_maker(level_values);
    }

    const std::string growtype_maximum_level =
        MakeIntList(script->growtype_maximum_level, 6);
    const std::string second_growtype_maximum_level =
        MakeSecondGrowtypeMaximumLevel(*script);
    const std::string skill_fitness_growtype =
        MakeIntList(script->skill_fitness_growtype);
    const std::string skill_fitness_second_growtype =
        MakeIntList(script->skill_fitness_second_growtype);
    const std::string pre_required_skill = MakeIntList(pre_required);
    const std::string consume_mp = MakeFloatList(script->consume_mp, 2);
    const std::string cool_time = MakeIntList(script->cool_time, 2);
    const std::string casting_time = MakeIntList(script->casting_time, 2);
    const std::string icon = MakeSkillIcon(escaped[4], script->icon_x,
                                           escaped[5], script->icon_y);
    const std::string skill_command_advantage =
        MakeIntPair(script->skill_command_advantage,
                    script->skill_command_advantage_value);
    const std::string static_data = MakeIntList(script->static_data);
    const std::string start_cool_time =
        MakeIntPair(script->start_cool_time,
                    script->start_cool_time_value);

    db_handle->set_query(
        "inSert into skill_info(job_index, skill_index, module_type, name, name2, basic_explain, skill_explain, purchase_cost, type, skill_class, growtype_maximum_level, second_growtype_maximum_level, skill_fitness_growtype, skill_fitness_second_growtype, consume_item, required_level, required_level_range, pre_required_skill, consume_mp, cool_time, casting_time, icon, command_key_explain, skill_command_advantage, static_data, level_info, start_cool_time) values(%d, %d, %d, '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
        job,
        script->skill_index,
        module_type,
        escaped[0],
        escaped[1],
        escaped[2],
        escaped[3],
        MakeIntList(script->purchase_cost).c_str(),
        script->type,
        script->skill_class,
        growtype_maximum_level.c_str(),
        second_growtype_maximum_level.c_str(),
        skill_fitness_growtype.c_str(),
        skill_fitness_second_growtype.c_str(),
        consume_item_maker.c_str(),
        script->required_level,
        script->required_level_range,
        pre_required_skill.c_str(),
        consume_mp.c_str(),
        cool_time.c_str(),
        casting_time.c_str(),
        icon.c_str(),
        escaped[6],
        skill_command_advantage.c_str(),
        static_data.c_str(),
        level_info_maker.c_str(),
        start_cool_time.c_str());
    return db_handle->exec(true) == true;
}

}  // namespace sync_script
