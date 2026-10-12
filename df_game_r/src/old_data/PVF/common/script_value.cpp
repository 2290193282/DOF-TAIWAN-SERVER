#include "old_data/PVF/common/script_value.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string.h>


typedef char ScriptValue_Size_Must_Be_0x14[(sizeof(ScriptValue) == 0x14) ? 1 : -1];

// IDA_EA: 0x88C4200
// IDA_NAME: _ZN11ScriptValueC2Ev
// DEMANGLED: ScriptValue::ScriptValue(void)
ScriptValue::ScriptValue() : value_type(3), values() {}

// IDA_EA: 0x83661E2
// IDA_NAME: _ZN11ScriptValueC2ERKS_
// DEMANGLED: ScriptValue::ScriptValue(ScriptValue const&)
ScriptValue::ScriptValue(const ScriptValue& other) : value_type(other.value_type) {
    for (int i = 0; i <= 3; ++i) {
        values[i] = other.values[i];
    }
}

// IDA_EA: 0x8366220
// IDA_NAME: _ZN11ScriptValueaSERKS_
// DEMANGLED: ScriptValue::operator=(ScriptValue const&)
ScriptValue& ScriptValue::operator=(const ScriptValue& other) {
    value_type = other.value_type;
    for (int i = 0; i <= 3; ++i) {
        values[i] = other.values[i];
    }
    return *this;
}

// IDA_EA: 0x88C29A2
// IDA_NAME: _Z26getActiveStatusToEnumValueRSs
// DEMANGLED: getActiveStatusToEnumValue(std::string &)
int getActiveStatusToEnumValue(std::string& value) {
    if (value == "slow") {
        return 0;
    }
    if (value == "freeze") {
        return 1;
    }
    if (value == "poison") {
        return 2;
    }
    if (value == "stun") {
        return 3;
    }
    if (value == "curse") {
        return 4;
    }
    if (value == "blind") {
        return 5;
    }
    if (value == "lightning") {
        return 6;
    }
    if (value == "stone") {
        return 7;
    }
    if (value == "sleep") {
        return 8;
    }
    if (value == "burn") {
        return 9;
    }
    if (value == "weapon break") {
        return 10;
    }
    if (value == "bleeding") {
        return 11;
    }
    if (value == "haste") {
        return 12;
    }
    if (value == "bless") {
        return 13;
    }
    if (value == "element") {
        return 14;
    }
    if (value == "confuse") {
        return 15;
    }
    if (value == "hold") {
        return 16;
    }
    if (value == "armor break") {
        return 17;
    }
    if (value == "any") {
        return 18;
    }
    if (value == "all") {
        return 19;
    }
    return -1;
}

// IDA_EA: 0x88C2C34
// IDA_NAME: _Z20getTargetToEnumValueRSs
// DEMANGLED: getTargetToEnumValue(std::string &)
int getTargetToEnumValue(std::string& value) {
    if (value == "myself") {
        return 0;
    }
    if (value == "enemy") {
        return 1;
    }
    if (value == "party") {
        return 2;
    }
    if (value == "random party") {
        return 3;
    }
    if (value == "all enemy") {
        return 4;
    }
    if (value == "enemy hp max") {
        return 5;
    }
    if (value == "enemy hp min") {
        return 6;
    }
    if (value == "enemy near") {
        return 7;
    }
    if (value == "enemy far") {
        return 8;
    }
    if (value == "random enemy") {
        return 9;
    }
    return 10;
}

// IDA_EA: 0x89BBE1E
// IDA_NAME: _Z32getActivePropertyTypeToEnumValueRSs
// DEMANGLED: getActivePropertyTypeToEnumValue(std::string &)
int getActivePropertyTypeToEnumValue(std::string& value) {
    if (value == "slow resistance") return 0;
    if (value == "freeze resistance") return 1;
    if (value == "poison resistance") return 2;
    if (value == "stun resistance") return 3;
    if (value == "curse resistance") return 4;
    if (value == "blind resistance") return 5;
    if (value == "lightning resistance") return 6;
    if (value == "stone resistance") return 7;
    if (value == "sleep resistance") return 8;
    if (value == "burn resistance") return 9;
    if (value == "weapon break resistance") return 10;
    if (value == "bleeding resistance") return 11;
    if (value == "piercing resistance") return 12;
    if (value == "stuck resistance") return 13;
    if (value == "confuse resistance") return 14;
    if (value == "hold resistance") return 15;
    return 17;
}

// IDA_EA: 0x89BCB06
// IDA_NAME: _Z18getBuffToEnumValueRSs
// DEMANGLED: getBuffToEnumValue(std::string &)
int getBuffToEnumValue(std::string& value) {
    if (value == "superarmor") {
        return 0;
    }
    if (value == "unbreakable") {
        return 1;
    }
    if (value == "camoflage") {
        return 2;
    }
    return 3;
}

// IDA_EA: 0x89BCDE0
// IDA_NAME: _Z30getExcepttionStatusToEnumValueRSs
// DEMANGLED: getExcepttionStatusToEnumValue(std::string &)
int getExcepttionStatusToEnumValue(std::string& value) {
    if (value == "silence") {
        return 1;
    }
    if (value == "disease") {
        return 3;
    }
    if (value == "mindcontrol") {
        return 2;
    }
    if (value == "all") {
        return 5;
    }
    return -1;
}

// IDA_EA: 0x88BD381
// IDA_NAME: _Z15scanScriptValueR11ScriptValue
// DEMANGLED: scanScriptValue(ScriptValue &)
int scanScriptValue(ScriptValue& value) {
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_HEXADECIMAL) {
        value.value_type = 0;
        const float raw = static_cast<float>(g_dnfLex.getHexDecimal());
        memcpy(&value.values[0], &raw, sizeof(raw));
        return 1;
    }
    if (token == DNF_TOK_DECIMAL) {
        value.value_type = 0;
        const float raw = static_cast<float>(g_dnfLex.getDecimal());
        memcpy(&value.values[0], &raw, sizeof(raw));
        return 1;
    }
    if (token == DNF_TOK_FLOAT) {
        value.value_type = 0;
        const float raw = g_dnfLex.getFloat();
        memcpy(&value.values[0], &raw, sizeof(raw));
        return 1;
    }
    if (token != DNF_TOK_STRING) {
        return 0;
    }

    std::string text(g_dnfLex.getString());
    int lookup = -1;
    if (text == "skill level info") {
        lookup = 1652;
    } else if (text == "object number") {
        lookup = 1670;
    }

    if (lookup == 1652) {
        value.value_type = 1;
        float raw0 = 0.0f;
        float raw1 = 0.0f;
        float raw2 = 0.0f;
        if (ScanFloat(&raw0) == 1) {
            memcpy(&value.values[0], &raw0, sizeof(raw0));
            if (ScanFloat(&raw1) == 1 && ScanFloat(&raw2) == 1) {
                memcpy(&value.values[1], &raw1, sizeof(raw1));
                memcpy(&value.values[2], &raw2, sizeof(raw2));
                return 1;
            }
        }
        return 0;
    }

    if (lookup == 1670) {
        value.value_type = 2;
        if (ScanStr(&text) == 1) {
            const float target =
                static_cast<float>(
                    static_cast<unsigned int>(getTargetToEnumValue(text)));
            memcpy(&value.values[0], &target, sizeof(target));
            float raw1 = 0.0f;
            if (ScanFloat(&raw1) == 1) {
                memcpy(&value.values[1], &raw1, sizeof(raw1));
                if (ScanStr(&text) == 1) {
                    const float active =
                        static_cast<float>(
                            getActiveStatusToEnumValue(text));
                    memcpy(&value.values[2], &active, sizeof(active));
                    float raw3 = 0.0f;
                    if (ScanFloat(&raw3) == 1) {
                        memcpy(&value.values[3], &raw3, sizeof(raw3));
                        return 1;
                    }
                }
            }
        }
        return 0;
    }

    return 0;
}

// IDA_EA: 0x88BD8E6
// IDA_NAME: _Z15scanScriptValueRSt6vectorIfSaIfEE
// DEMANGLED: scanScriptValue(std::vector<float,std::allocator<float>> &)
int scanScriptValue(std::vector<float>& values) {
    ScriptValue value;
    if (scanScriptValue(value) != 1) {
        return 0;
    }
    pushScriptValue(values, value);
    return 1;
}

// IDA_EA: 0x88BD929
// IDA_NAME: _Z15pushScriptValueRSt6vectorIfSaIfEER11ScriptValue
// DEMANGLED: pushScriptValue(std::vector<float,std::allocator<float>> &,ScriptValue &)
bool pushScriptValue(std::vector<float>& values, ScriptValue& value) {
    const float value_type = static_cast<float>(value.value_type);
    values.push_back(value_type);
    for (int index = 0; index <= 3; ++index) {
        float raw = 0.0f;
        memcpy(&raw, &value.values[index], sizeof(raw));
        values.push_back(raw);
    }
    return false;
}

// IDA_EA: 0x88BD984
// IDA_NAME: _Z14getScriptValueR11ScriptValueRSt6vectorIfSaIfEEi
// DEMANGLED: getScriptValue(ScriptValue &,std::vector<float,std::allocator<float>> &,int)
bool getScriptValue(ScriptValue& value,
                    std::vector<float>& values,
                    int index) {
    bool result = values.size() <= static_cast<std::size_t>(index + 4);
    if (!result) {
        value.value_type = static_cast<int>(values.at(index));
        for (int value_index = 0; value_index <= 3; ++value_index) {
            const float raw = values.at(value_index + index + 1);
            memcpy(&value.values[value_index], &raw, sizeof(raw));
        }
    }
    return result;
}
