#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/script_value.h"
#include "common/manager/change_status_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/striker_public_abi.h"

#include <cstring>
#include <string>
#include <vector>

// IDA_EA: 0x8991229
// IDA_NAME: _Z21GetApplyStatusInRangePSt6vectorIfSaIfEE
// DEMANGLED: GetApplyStatusInRange(std::vector<float,std::allocator<float>> *)
int GetApplyStatusInRange(std::vector<float>* values) {
    if (!values) {
        return 0;
    }

    int code_bits = 1109131264;
    float code = 0.0f;
    std::memcpy(&code, &code_bits, sizeof(code));
    values->push_back(code);
    std::string status;
    if (ScanStr(&status) != 1) {
        return 0;
    }
    const float status_type =
        static_cast<float>(getChangeStatusType(status));
    if (ScanStr(&status) != 1) {
        return 0;
    }
    const float is_percent = status == "%" ? 1.0f : 0.0f;
    bool success = false;
    const float first = static_cast<float>(ScanInt(&success));
    if (!success) {
        return 0;
    }
    const float second = static_cast<float>(ScanInt(&success));
    if (!success) {
        return 0;
    }
    values->push_back(status_type);
    values->push_back(is_percent);
    values->push_back(first);
    values->push_back(second);
    return 1;
}

// IDA_EA: 0x899CD6C
// IDA_NAME: _Z6scanIfP22EquipmentParameterInfoi
// DEMANGLED: scanIf(EquipmentParameterInfo *,int)
void scanIf(EquipmentParameterInfo* info, int unused) {
    (void)unused;
    std::string tag;
    std::vector<int> values;
    while (ScanType(tag, true) == 1) {
        values.clear();
        if (tag == "[/if]") {
            break;
        }
        readCondtion(info, tag, values);
        values.empty();
    }
}

// IDA_EA: 0x899CE3B
// IDA_NAME: _Z12readCondtionP22EquipmentParameterInfoRSsRSt6vectorIiSaIiEE
// DEMANGLED: readCondtion(EquipmentParameterInfo *,std::string &,std::vector<int,std::allocator<int>> &)
void readCondtion(EquipmentParameterInfo* info,
                  std::string& tag,
                  std::vector<int>& values) {
    std::string text;
    bool success = false;

    if (tag == "[target active status on]" ||
        tag == "[target active status]" ||
        tag == "[my active status on]" ||
        tag == "[my active status]" || tag == "[target state]" ||
        tag == "[my state]" || tag == "[target prev state]" ||
        tag == "[my prev state]") {
        if (ScanStr(&text) == 1) {
            values.push_back(getChangeStatusType(text));
        }
        return;
    }
    if (tag == "[attack success]" || tag == "[after attack]" ||
        tag == "[use command]" || tag == "[event hit]" ||
        tag == "[event attack success]" || tag == "[cooltime group]" ||
        tag == "[cooltime]" || tag == "[start cooltime]" ||
        tag == "[my appendage]" || tag == "[combo]" ||
        tag == "[set my state]" || tag == "[target cooltime]") {
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[skill cooltime]") {
        if (ScanStr(&text) == 1) {
            values.push_back(ScanInt(&success));
        }
        return;
    }
    if (tag == "[casting]" || tag == "[hit]" || tag == "[revive]" ||
        tag == "[aerial]" || tag == "[party death]" ||
        tag == "[overkill]" || tag == "[disable pvp]" ||
        tag == "[target death]") {
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[character]" || tag == "[element]" ||
        tag == "[attack condition]" || tag == "[attack type]" ||
        tag == "[target type]" || tag == "[target grade]" ||
        tag == "[distance]") {
        ScanStr(&text);
        return;
    }
    if (tag == "[time]") {
        for (int index = 0; index < 3; ++index) {
            values.push_back(ScanInt(&success));
        }
        return;
    }
    if (tag == "[attacker]" || tag == "[event use skill]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[skill]") {
        bool is_type = false;
        while (ScanTypeOrStr(is_type, text, true) == 1) {
            // ScanTypeOrStr reports success for the physical [/skill] TYPE.
            // Testing is_type only in the while condition still executes this
            // body once for that closing tag and lets ScanInt consume the next
            // condition tag (3choroset66 loses [attack success] that way).
            if (is_type) {
                return;
            }
            values.push_back(ScanInt(&success));
            if (!success) {
                return;
            }
        }
        return;
    }
    if (tag == "[use skill]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[skill frame trigger]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        if (ScanStr(&text) == 1 && info) {
            values.push_back(info->addString(text));
        }
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[skill frame state]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        if (ScanStr(&text) == 1 && info) {
            values.push_back(info->addString(text));
        }
        values.push_back(ScanInt(&success));
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[change status in range]") {
        // IDA 0x899CE3B (opcode 48) consumes exactly one status selector
        // followed by two integers.  The 0828 scripts close the surrounding
        // [if] immediately after this tuple; probing for a second string
        // consumes that [/if] token and lets scanIf run into the following
        // [then] block.
        values.push_back(48);
        ScanStr(&text);
        int status_selector = 0;
        if (text == "mp") {
            status_selector = 1;
        } else if (text == "vp") {
            status_selector = 2;
        }
        values.push_back(status_selector);
        values.push_back(ScanInt(static_cast<bool*>(0)));
        values.push_back(ScanInt(static_cast<bool*>(0)));
        return;
    }
    if (tag == "[change status]") {
        while (ScanStr(&text) == 1 && text != "end") {
            if (ScanStr(&text) != 1 ||
                ScanStr(&text) != 1) {
                return;
            }
            values.push_back(ScanInt(&success));
        }
        return;
    }
    if (tag == "[stat change]" || tag == "[target stat]") {
        ScanStr(&text);
        ScanStr(&text);
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[check damage]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[party count]") {
        ScanStr(&text);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[target check]") {
        bool is_type = false;
        while (ScanTypeOrStr(is_type, text, true) == 1) {
            // IDA readCondtion @0x899CE3B, opcode 18: the TYPE returned by
            // ScanTypeOrStr terminates the list before the paired integer is
            // read.  Reading the integer from the closing TYPE consumes the
            // next condition tag, exactly like the 3choroset66 [skill] bug.
            if (is_type) {
                return;
            }
            values.push_back(ScanInt(&success));
            if (!success) {
                return;
            }
        }
        return;
    }
    if (tag == "[module]") {
        int module = scanModuleType();
        while (module != 17) {
            values.push_back(module);
            module = scanModuleType();
        }
        return;
    }
    if (tag == "[dungeon check]") {
        bool is_type = false;
        while (!is_type) {
            const int dungeon =
                ScanTypeOrInt(is_type, text, success, true);
            // IDA readCondtion @0x899CE3B, opcode 46 tests the TYPE flag
            // before publishing the returned integer slot.
            if (is_type) {
                break;
            }
            if (!success) {
                return;
            }
            values.push_back(dungeon);
        }
        return;
    }
    if (tag == "[attack success effect animation]") {
        const int count = ScanInt(&success);
        for (int index = 0; success && index < count; ++index) {
            ScanStr(&text);
            ScanInt(&success);
            ScanInt(&success);
        }
        return;
    }
    if (tag == "[slot option]") {
        ScanInt(&success);
        ScanInt(&success);
        ScanStr(&text);
        ScanInt(&success);
        return;
    }
    if (tag == "[keep my state]") {
        ScanStr(&text);
        ScanInt(&success);
        return;
    }
    if (tag == "[is stat]") {
        // IDA readCondtion map key 4470 / opcode 54 consumes target, target
        // index, status,
        // comparison operator, then threshold.  The missing operator read made
        // ScanInt consume strings such as "=>"; scanIf then crossed the
        // threshold and swallowed the following [cooltime] TYPE before
        // returning.  Set-ability parsing consequently reached EOF with its
        // scope still open (first seen in 100050050.equ).
        if (ScanStr(&text) != 1) {
            return;
        }
        values.push_back(ScanInt(&success));
        if (!success || ScanStr(&text) != 1) {
            return;
        }
        std::string comparison;
        if (ScanStr(&comparison) != 1) {
            return;
        }
        int comparison_code = 0;
        if (comparison == "<") {
            comparison_code = 1;
        } else if (comparison == "<=" || comparison == "=<") {
            comparison_code = 2;
        } else if (comparison == "=") {
            comparison_code = 3;
        } else if (comparison == ">=" || comparison == "=>") {
            comparison_code = 4;
        } else if (comparison == ">") {
            comparison_code = 5;
        } else {
            return;
        }
        values.push_back(comparison_code);
        values.push_back(ScanInt(&success));
        return;
    }
    if (tag == "[equipment upgrade]") {
        while (ScanStr(&text) == 1) {
            if (ScanStr(&text) != 1) {
                return;
            }
            ScanInt(&success);
            if (!success) {
                return;
            }
            ScanInt(&success);
            if (!success) {
                return;
            }
        }
        return;
    }
    if (tag == "[remove equip]" || tag == "[is awakening state]") {
        return;
    }
    if (tag == "[timing]") {
        ScanStr(&text);
    }
}

// IDA_EA: 0x89A123C
// IDA_NAME: _Z8scanThenSsP22EquipmentParameterInfoi
// DEMANGLED: scanThen(std::string,EquipmentParameterInfo *,int)
int scanThen(std::string filename, EquipmentParameterInfo* info, int unused) {
    std::vector<std::vector<float> > parsed_actions;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/then]") {
            return 1;
        }
        std::vector<float> action;
        if (readAction(filename, info, tag, action, unused) != 1) {
            return 0;
        }
        if (!action.empty()) {
            parsed_actions.push_back(action);
        }
    }
    return 0;
}

// IDA_EA: 0x89A13E5
// IDA_NAME: _Z10readActionRKSsP22EquipmentParameterInfoRSsRSt6vectorIfSaIfEEi
// DEMANGLED: readAction(std::string const&,EquipmentParameterInfo *,std::string &,std::vector<float,std::allocator<float>> &,int)
int readAction(const std::string& filename,
               EquipmentParameterInfo* info,
               std::string& tag,
               std::vector<float>& values,
               int unused) {
    (void)filename;
    (void)unused;

    std::string text;
    bool success = false;
    int value = 0;
    float float_value = 0.0f;

    if (tag == "[consume item]") {
        value = ScanInt(&success);
        if (!success) return 0;
        values.push_back(static_cast<float>(value));
        value = ScanInt(&success);
        if (!success) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[target]") {
        if (ScanStr(&text) != 1) return 0;
        value = ScanInt(&success);
        if (!success) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[duration]" || tag == "[equipment duration]") {
        return scanScriptValue(values);
    }
    if (tag == "[delay]" || tag == "[reduce probability at pvp module]" ||
        tag == "[reduce probability at pvp type module]" ||
        tag == "[reduce probability to human armor at pvp module]" ||
        tag == "[reduce probability to human armor at pvp type module]" ||
        tag == "[reduce duration at pvp module]" ||
        tag == "[reduce duration at pvp type module]" ||
        tag == "[reduce duration to human armor]" ||
        tag == "[reduce duration to human armor at pvp module]" ||
        tag == "[reduce duration to human armor at pvp type module]") {
        value = ScanInt(&success);
        if (!success) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[probability]") {
        return ScanFloat(&float_value);
    }
    if (tag == "[skill duration]") {
        if (ScanInt(&value) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[reduce probability to human armor]" ||
        tag == "[appendage]" || tag == "[appendage unique]" ||
        tag == "[aura]" || tag == "[thorn]" || tag == "[weakness]" ||
        tag == "[disenchant]") {
        value = ScanInt(&success);
        if (!success) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[aura active]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanFloat(&float_value) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(float_value);
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[active status]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) {
            return 0;
        }
        if (text == "blind" || text == "slow" || text == "haste") {
            for (int index = 0; index < 2; ++index) {
                if (ScanFloat(&float_value) != 1) return 0;
                values.push_back(float_value);
            }
            return 1;
        }
        if (text == "burn" || text == "bless" || text == "curse") {
            for (int index = 0; index < 4; ++index) {
                if (ScanFloat(&float_value) != 1) return 0;
                values.push_back(float_value);
            }
            return 1;
        }
        if (text == "stone" || text == "confuse" || text == "weakness" ||
            text == "armor break" || text == "weapon break" ||
            text == "poison" || text == "lightning" || text == "bleeding") {
            if (ScanFloat(&float_value) != 1) return 0;
            values.push_back(float_value);
            return 1;
        }
        // IDA 0x89A13E5 accepts valid status names with no status-specific
        // tail (for example freeze) after consuming the common type/level
        // pair.  Only the status families above consume extra values.
        return 1;
    }
    if (tag == "[apply status in range]") {
        return GetApplyStatusInRange(&values);
    }
    if (tag == "[stat by condition]") {
        return GetStatByCondition(&values);
    }
    if (tag == "[stat]") {
        return GetStat(&values);
    }
    if (tag == "[stat level replace]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[active status resistance]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1 ||
            ScanStr(&text) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[element resistance]" || tag == "[add absolute damage]" ||
        tag == "[add absolute defense]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[add absolute defense percent]") {
        if (ScanStr(&text) != 1 ||
            ScanFloat(&float_value) != 1) return 0;
        values.push_back(float_value);
        return 1;
    }
    if (tag == "[buff]" || tag == "[elemental weapon]") {
        if (ScanStr(&text) != 1) return 0;
        return 1;
    }
    if (tag == "[reduction]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) {
            return 0;
        }
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[knockback]") {
        return ScanInt(&success) != 0 ? 1 : 0;
    }
    if (tag == "[set hp]") {
        return ScanFloat(&float_value);
    }
    if (tag == "[damage rate]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[active status duration]" ||
        tag == "[active status buff event type]" ||
        tag == "[then probability]") {
        return ScanFloat(&float_value);
    }
    if (tag == "[passive object]") {
        bool is_type = false;
        while (1) {
            const int first = ScanTypeOrInt(is_type, text, success, true);
            if (is_type && text == "[/passive object]") {
                return 1;
            }
            if (is_type || !success) {
                return 0;
            }
            values.push_back(static_cast<float>(first));
            for (int index = 0; index < 5; ++index) {
                if (ScanInt(&value) != 1) return 0;
                values.push_back(static_cast<float>(value));
            }
            if (ScanStr(&text) != 1) return 0;
        }
    }
    if (tag == "[summon monster]" || tag == "[summon apc]") {
        for (int index = 0; index < 3; ++index) {
            if (ScanInt(&value) != 1) return 0;
            values.push_back(static_cast<float>(value));
        }
        return 1;
    }
    if (tag == "[event summon monster]") {
        for (int index = 0; index < 4; ++index) {
            if (ScanInt(&value) != 1) return 0;
            values.push_back(static_cast<float>(value));
        }
        return 1;
    }
    if (tag == "[restore]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        if (ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[skill]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        if (ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[perform skill]") {
        if (ScanStr(&text) != 1) return 0;
        for (int index = 0; index < 3; ++index) {
            if (ScanInt(&value) != 1) return 0;
            values.push_back(static_cast<float>(value));
        }
        return 1;
    }
    if (tag == "[end skill]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[increase damage]" || tag == "[increase critical damage]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[skill levelup]") {
        return GetSkillLevelUp(&values) ? 1 : 0;
    }
    if (tag == "[skill data up]") {
        if (!info) return 0;
        std::vector<std::vector<EquipmentSkillDataUp> >* data =
            &info->skill_data_up_1e8;
        data->resize(data->size() + 1);
        importSkillDataUp(data->back());
        values.push_back(static_cast<float>(data->size() - 1));
        return 1;
    }
    if (tag == "[speech on]") {
        if (ScanInt(&value) != 1 || ScanInt(&value) != 1 ||
            ScanStr(&text) != 1) return 0;
        if (info) {
            info->strings_1f4.push_back(text);
        }
        if (ScanType(text, true) != 1 || text != "[/speech on]") return 0;
        return 1;
    }
    if (tag == "[replace avatar ani]") {
        if (!info) return 0;
        std::vector<std::pair<int, int> >* animations =
            &info->conditional_values_468;
        bool is_type = false;
        while (ScanTypeOrStr(is_type, text, true) == 1) {
            if (is_type) {
                return text == "[/replace avatar ani]" ? 1 : 0;
            }
            if (ScanInt(&value) != 1) return 0;
            animations->push_back(
                std::make_pair(GetEquipmentTypeStringToEnum(text), value));
        }
        return 0;
    }
    if (tag == "[skill super armor]" || tag == "[skill unbreakable]") {
        if (ScanStr(&text) != 1 || ScanFloat(&float_value) != 1) {
            return 0;
        }
        values.push_back(float_value);
        return 1;
    }
    if (tag == "[stat change by stat]") {
        if (ScanStr(&text) != 1 || ScanInt(&value) != 1 ||
            ScanStr(&text) != 1 || ScanInt(&value) != 1 ||
            ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1 || ScanInt(&value) != 1) {
            return 0;
        }
        return 1;
    }
    if (tag == "[add stat at once]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        return 1;
    }
    if (tag == "[add skill summon]") {
        for (int index = 0; index < 3; ++index) {
            if (ScanInt(&value) != 1) return 0;
            values.push_back(static_cast<float>(value));
        }
        return ScanType(text, true);
    }
    if (tag == "[swap stat]") {
        if (ScanStr(&text) != 1 || ScanStr(&text) != 1 ||
            ScanInt(&value) != 1 || ScanStr(&text) != 1 ||
            ScanFloat(&float_value) != 1) return 0;
        values.push_back(static_cast<float>(value));
        values.push_back(float_value);
        return 1;
    }
    if (tag == "[skill cooltime reset]") {
        if (ScanFloat(&float_value) != 1 || ScanStr(&text) != 1 ||
            ScanFloat(&float_value) != 1) return 0;
        values.push_back(float_value);
        return 1;
    }

    return 0;
}
