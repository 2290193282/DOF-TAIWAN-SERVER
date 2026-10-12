#include "old_game/appearance/avatar/compound_avatar_value_types.h"

#include <string>
#include <utility>

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/equipment/upgrade_table.h"

namespace {

int GetCompoundAvatarPartType(const std::string& type) {
    if (type == "hat") {
        return 0;
    }
    if (type == "hair") {
        return 1;
    }
    if (type == "face") {
        return 2;
    }
    if (type == "neck") {
        return 6;
    }
    if (type == "coat") {
        return 3;
    }
    if (type == "pants") {
        return 4;
    }
    if (type == "belt") {
        return 7;
    }
    if (type == "shoes") {
        return 5;
    }
    if (type == "skin") {
        return 8;
    }
    if (type == "aurora") {
        return 9;
    }
    return 22;
}

int GetCompoundRareAvatarType(const std::string& type) {
    if (type == "[hat rare avatar]") {
        return 0;
    }
    if (type == "[hair rare avatar]") {
        return 1;
    }
    if (type == "[face rare avatar]") {
        return 2;
    }
    if (type == "[neck rare avatar]") {
        return 6;
    }
    if (type == "[coat rare avatar]") {
        return 3;
    }
    if (type == "[pants rare avatar]") {
        return 4;
    }
    if (type == "[belt rare avatar]") {
        return 7;
    }
    if (type == "[shoes rare avatar]") {
        return 5;
    }
    if (type == "[skin rare avatar]") {
        return 8;
    }
    if (type == "[aurora rare avatar]") {
        return 9;
    }
    return 22;
}

unsigned int& AncientRareTotal(STAncientRareAvatarCube& cube, int part) {
    switch (part) {
    case 0: return cube.grade0_count;
    case 1: return cube.grade1_count;
    case 2: return cube.grade2_count;
    case 3: return cube.grade3_count;
    case 4: return cube.grade4_count;
    case 5: return cube.grade5_count;
    case 6: return cube.grade6_count;
    case 7: return cube.grade7_count;
    case 8: return cube.grade8_count;
    default: return cube.grade9_count;
    }
}

CompoundAvatarItemList& AncientRareItems(STAncientRareAvatarCube& cube,
                                         int part) {
    switch (part) {
    case 0: return cube.grade0_items;
    case 1: return cube.grade1_items;
    case 2: return cube.grade2_items;
    case 3: return cube.grade3_items;
    case 4: return cube.grade4_items;
    case 5: return cube.grade5_items;
    case 6: return cube.grade6_items;
    case 7: return cube.grade7_items;
    case 8: return cube.grade8_items;
    default: return cube.grade9_items;
    }
}

bool IsAncientRareAvatarTag(const std::string& type) {
    return type == "[hat rare avatar]" ||
           type == "[hair rare avatar]" ||
           type == "[face rare avatar]" ||
           type == "[neck rare avatar]" ||
           type == "[coat rare avatar]" ||
           type == "[pants rare avatar]" ||
           type == "[belt rare avatar]" ||
           type == "[shoes rare avatar]";
}

}  // namespace

// IDA_EA: 0x8902B8E
// IDA_NAME: _Z21setCompoundAvatarDataP17tagCompoundAvatar18ENUM_EQUIPMENTTYPE
// DEMANGLED: setCompoundAvatarData(tagCompoundAvatar *,ENUM_EQUIPMENTTYPE)
int setCompoundAvatarData(tagCompoundAvatar* avatar,
                          ENUM_EQUIPMENTTYPE equipment_type) {
    const int part = static_cast<int>(equipment_type);
    bool success = false;

    int count = ScanInt(&success);
    if (!success) {
        return 0;
    }
    avatar->normal_avatar_items[part].reserve(count);
    for (int index = 0; index < count; ++index) {
        const int item_id = ScanInt(&success);
        if (!success) {
            return 0;
        }
        const int rate = ScanInt(&success);
        if (!success) {
            return 0;
        }
        avatar->normal_avatar_total_rate_by_part[part] += rate;
        avatar->normal_avatar_items[part].push_back(
            std::make_pair(item_id, rate));
    }

    count = ScanInt(&success);
    if (!success) {
        return 0;
    }
    avatar->rare_avatar_items[part].reserve(count);
    for (int index = 0; index < count; ++index) {
        const int item_id = ScanInt(&success);
        if (!success) {
            return 0;
        }
        const int rate = ScanInt(&success);
        if (!success) {
            return 0;
        }
        avatar->rare_avatar_total_rate_by_part[part] += rate;
        avatar->rare_avatar_items[part].push_back(std::make_pair(item_id,
                                                                   rate));
    }

    while (true) {
        const int item_id = ScanInt(&success);
        if (!success) {
            break;
        }
        const int rate = ScanInt(&success);
        if (!success) {
            return 0;
        }
        avatar->upper_rare_avatar_total_rate_by_part[part] += rate;
        avatar->upper_rare_avatar_items[part].push_back(
            std::make_pair(item_id, rate));
    }
    return 1;
}

// IDA_EA: 0x8902E78
// IDA_NAME: _Z25setCompoundRareAvatarDataP17tagCompoundAvatar18ENUM_EQUIPMENTTYPE
// DEMANGLED: setCompoundRareAvatarData(tagCompoundAvatar *,ENUM_EQUIPMENTTYPE)
int setCompoundRareAvatarData(tagCompoundAvatar* avatar,
                              ENUM_EQUIPMENTTYPE equipment_type) {
    const int part = static_cast<int>(equipment_type);
    if (part == 22) {
        return 0;
    }

    bool success = false;
    while (true) {
        const int item_id = ScanInt(&success);
        if (!success) {
            break;
        }
        const int rate = ScanInt(&success);
        if (!success) {
            return 0;
        }
        AncientRareTotal(avatar->ancient_rare_cube, part) += rate;
        AncientRareItems(avatar->ancient_rare_cube, part).push_back(
            std::make_pair(item_id, rate));
    }
    return 1;
}

// IDA_EA: 0x8902F42
// IDA_NAME: _Z25getCompoundRareAvatarTypeRKSs
// DEMANGLED: getCompoundRareAvatarType(std::string const&)
int getCompoundRareAvatarType(const std::string& type) {
    return GetCompoundRareAvatarType(type);
}

// IDA_EA: 0x890308A
// IDA_NAME: _Z26getCompoundAvatarPartsTypeRKSs
// DEMANGLED: getCompoundAvatarPartsType(std::string const&)
int getCompoundAvatarPartsType(const std::string& type) {
    return GetCompoundAvatarPartType(type);
}

// IDA_EA: 0x89031D2
// IDA_NAME: _Z29importCompoundAvatarTableCorePKcS0_P22tagCompoundAvatarTable
// DEMANGLED: importCompoundAvatarTableCore(char const*,char const*,tagCompoundAvatarTable *)
int importCompoundAvatarTableCore(const char* directory,
                                  const char* filename,
                                  tagCompoundAvatarTable* table) {
    if (!loadRDARScriptFile(directory, filename)) {
        return 3870;
    }

    tagCompoundAvatarTable* current_table = 0;
    tagCompoundAvatar* current_avatar = 0;
    std::string type;
    std::string value;
    bool success = false;

    while (ScanType(type, true) == 1) {
        if (type == "[character type]") {
            if (ScanStr(&value) != 1) {
                return 3885;
            }
            const int job = getCharacterJob(value.c_str());
            if (job == oldsrv::config::kCharacterJobCommon) {
                return 3890;
            }
            current_table = table + job;
        } else if (type == "[grade]") {
            const int grade = ScanInt(&success);
            if (!success) {
                return 3895;
            }
            if (current_table == 0) {
                return 3897;
            }
            tagCompoundAvatar empty_avatar;
            current_table->entries.resize(grade + 1, empty_avatar);
            current_avatar = &current_table->entries[grade];
            current_avatar->Clean();
            current_avatar->grade = grade;
        } else if (type == "[rare rate]") {
            while (ScanStr(&value) == 1) {
                const int rate = ScanInt(&success);
                if (!success) {
                    return 3912;
                }
                const int part = GetCompoundAvatarPartType(value);
                if (part == 22 || current_avatar == 0) {
                    return 3916;
                }
                current_avatar->rare_rate_by_part[part] = rate;
            }
        } else if (type == "[upper grade]") {
            if (current_avatar == 0) {
                return 3923;
            }
            current_avatar->upper_grade = ScanInt(&success);
            if (!success) {
                return 3926;
            }
        } else if (type == "[upper rare rate]") {
            if (current_avatar == 0) {
                return 3931;
            }
            while (ScanStr(&value) == 1) {
                const int rate = ScanInt(&success);
                if (!success) {
                    return 3939;
                }
                const int part = GetCompoundAvatarPartType(value);
                if (part == 22) {
                    return 3943;
                }
                current_avatar->upper_rare_rate_by_part[part] = rate;
            }
        } else if (type == "[hat avatar]") {
            if (current_avatar == 0) return 3950;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(0)) != 1) return 3953;
        } else if (type == "[hair avatar]") {
            if (current_avatar == 0) return 3958;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(1)) != 1) return 3961;
        } else if (type == "[face avatar]") {
            if (current_avatar == 0) return 3966;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(2)) != 1) return 3969;
        } else if (type == "[neck avatar]") {
            if (current_avatar == 0) return 3974;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(6)) != 1) return 3977;
        } else if (type == "[coat avatar]") {
            if (current_avatar == 0) return 3982;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(3)) != 1) return 3985;
        } else if (type == "[pants avatar]") {
            if (current_avatar == 0) return 3990;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(4)) != 1) return 3993;
        } else if (type == "[belt avatar]") {
            if (current_avatar == 0) return 3998;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(7)) != 1) return 4001;
        } else if (type == "[shoes avatar]") {
            if (current_avatar == 0) return 4006;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(5)) != 1) return 4009;
        } else if (type == "[skin avatar]") {
            if (current_avatar == 0) return 4014;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(8)) != 1) return 4017;
        } else if (type == "[aurora avatar]") {
            if (current_avatar == 0) return 4023;
            if (setCompoundAvatarData(current_avatar,
                                      static_cast<ENUM_EQUIPMENTTYPE>(9)) != 1) return 4026;
        } else if (type == "[material]" || type == "[material_upper]" ||
                   type == "[material_lower]" || type == "[material_ancient]" ||
                   type == "[material_master]") {
            const int no_avatar_code =
                type == "[material]" ? 4042 :
                type == "[material_upper]" ? 4108 :
                type == "[material_lower]" ? 4131 :
                type == "[material_ancient]" ? 4154 : 4172;
            const int count_code =
                type == "[material]" ? 4045 :
                type == "[material_upper]" ? 4111 :
                type == "[material_lower]" ? 4134 :
                type == "[material_ancient]" ? 4157 : 4175;
            const int second_value_code =
                type == "[material]" ? 4052 :
                type == "[material_upper]" ? 4118 :
                type == "[material_lower]" ? 4141 :
                type == "[material_ancient]" ? 4164 : 4182;
            const int invalid_count_code =
                type == "[material]" ? 4057 :
                type == "[material_upper]" ? 4123 :
                type == "[material_lower]" ? 4146 :
                type == "[material_ancient]" ? 4169 : 4186;
            if (current_avatar == 0) {
                return no_avatar_code;
            }
            (void)ScanInt(&success);
            if (!success) {
                return count_code;
            }
            CompoundAvatarItemList* target =
                type == "[material]" ? &current_avatar->material_items :
                type == "[material_upper]" ? &current_avatar->material_upper_items :
                type == "[material_lower]" ? &current_avatar->material_lower_items :
                type == "[material_ancient]" ?
                    &current_avatar->ancient_rare_cube.common_items :
                    &current_avatar->material_master_items;
            while (true) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return second_value_code;
                }
                target->push_back(std::make_pair(item_id, item_count));
                if (item_count != 1) {
                    return invalid_count_code;
                }
            }
        } else if (type == "[ancient bind cube]") {
            if (current_avatar == 0) {
                return 4175;
            }
            while (ScanType(type, true) == 1 &&
                   type != "[/ancient bind cube]") {
                if (IsAncientRareAvatarTag(type) &&
                    setCompoundRareAvatarData(
                        current_avatar,
                        static_cast<ENUM_EQUIPMENTTYPE>(
                            GetCompoundRareAvatarType(type))) != 1) {
                    return 4189;
                }
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8903EA7
// IDA_NAME: _Z25importCompoundAvatarTablePKcS0_P22tagCompoundAvatarTable
// DEMANGLED: importCompoundAvatarTable(char const*,char const*,tagCompoundAvatarTable *)
int importCompoundAvatarTable(const char* directory,
                              const char* filename,
                              tagCompoundAvatarTable* table) {
    STScriptFileList script_files;
    if (loadRDARScriptListFile(&script_files, directory, filename)) {
        if (script_files.size() ==
            static_cast<size_t>(oldsrv::config::kCharacterJobCount)) {
            for (STScriptFileList::const_iterator it = script_files.begin();
                 it != script_files.end(); ++it) {
                if (importCompoundAvatarTableCore(directory, it->second.c_str(),
                                                  table)) {
                    return 4213;
                }
            }
            return 0;
        }
        return 4206;
    }
    return 4219;
}
