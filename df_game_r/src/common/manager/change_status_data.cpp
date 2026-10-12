#include "common/manager/change_status_data.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"


namespace {

struct ChangeStatusTypeEntry {
    const char* name;
    int type;
};

const ChangeStatusTypeEntry kChangeStatusTypes[] = {
    {"physical attack", 0},
    {"magical attack", 1},
    {"magical defense", 2},
    {"physical defense", 3},
    {"equipment physical attack", 4},
    {"physical equipment attack", 4},
    {"equipment physical defense", 5},
    {"physical equipment defense", 5},
    {"physical attack bonus", 6},
    {"magical attack bonus", 7},
    {"jump power", 8},
    {"attack speed", 10},
    {"move speed", 11},
    {"cast speed", 12},
    {"hp max", 13},
    {"hp regen rate", 14},
    {"magical critical hit rate", 16},
    {"physical critical hit", 15},
    {"physical critical hit rate", 15},
    {"magical critical hit", 16},
    {"physical back attack critical rate", 17},
    {"physical back attack critical", 17},
    {"magical back attack critical rate", 18},
    {"fire element", 20},
    {"fire resistance", 20},
    {"water element", 21},
    {"water resistance", 21},
    {"dark element", 22},
    {"dark resistance", 22},
    {"light element", 23},
    {"light resistance", 23},
    {"all elemental resistance", 24},
    {"all activestatus resistance", 26},
    {"stuck resistance", 27},
    {"mp regen rate", 29},
    {"extra speed rate", 30},
    {"equipment magical attack", 31},
    {"magical equipment attack", 31},
    {"equipment magical defense", 32},
    {"magical equipment defense", 32},
    {"stuck", 33},
    {"rigidity", 41},
    {"fire attack", 42},
    {"water attack", 43},
    {"dark attack", 44},
    {"light attack", 45},
    {"mp max", 48},
    {"skill cool time", 49},
    {"separate attack", 53},
    {"antievil", 55},
    {"physical absolute damage", 56},
    {"no breath time change", 57},
};

}  // namespace

// IDA_EA: 0x8366190
// IDA_NAME: _ZN16ChangeStatusDataC2ERKS_
// DEMANGLED: ChangeStatusData::ChangeStatusData(ChangeStatusData const&)
ChangeStatusData::ChangeStatusData(const ChangeStatusData& other)
    : change_status_type(other.change_status_type),
      is_percent(other.is_percent),
      value(other.value) {}

// IDA_EA: 0x83661B8
// IDA_NAME: _ZN16ChangeStatusDataaSERKS_
// DEMANGLED: ChangeStatusData::operator=(ChangeStatusData const&)
ChangeStatusData& ChangeStatusData::operator=(const ChangeStatusData& other) {
    change_status_type = other.change_status_type;
    is_percent = other.is_percent;
    value = other.value;
    return *this;
}

// IDA_EA: 0x89C2DD8
// IDA_NAME: _ZN16ChangeStatusDataC2Ev
// DEMANGLED: ChangeStatusData::ChangeStatusData(void)
ChangeStatusData::ChangeStatusData()
    : change_status_type(77), is_percent(0), value(0.0f) {}

// IDA_EA: 0x8366262
// IDA_NAME: _ZN12VariableStatC2ERKS_
// DEMANGLED: VariableStat::VariableStat(VariableStat const&)
VariableStat::VariableStat(const VariableStat& other)
    : status(other.status), script_value(other.script_value) {}

// IDA_EA: 0x8366294
// IDA_NAME: _ZN12VariableStataSERKS_
// DEMANGLED: VariableStat::operator=(VariableStat const&)
VariableStat& VariableStat::operator=(const VariableStat& other) {
    status = other.status;
    script_value = other.script_value;
    return *this;
}

// IDA_EA: 0x89C2DFC
// IDA_NAME: _ZN12VariableStatC2Ev
// DEMANGLED: VariableStat::VariableStat(void)
VariableStat::VariableStat() : status(), script_value() {}

// IDA_EA: 0x89D2EC8
// IDA_NAME: _ZN12VariableStatD2Ev
// DEMANGLED: VariableStat::~VariableStat()
VariableStat::~VariableStat() {}

// IDA_EA: 0x8892B0D
// IDA_NAME: _Z19getChangeStatusTypeRSs
// DEMANGLED: getChangeStatusType(std::string &)
int getChangeStatusType(std::string& value) {
    for (unsigned int index = 0;
         index < sizeof(kChangeStatusTypes) / sizeof(kChangeStatusTypes[0]);
         ++index) {
        if (value == kChangeStatusTypes[index].name) {
            return kChangeStatusTypes[index].type;
        }
    }
    return 77;
}

// IDA_EA: 0x88BDA1C
// IDA_NAME: _Z20scanChangeStatusDataR16ChangeStatusData
// DEMANGLED: scanChangeStatusData(ChangeStatusData &)
int scanChangeStatusData(ChangeStatusData& value) {
    std::string text;
    bool is_type = false;
    if (ScanTypeOrStr(is_type, text, true) != 1) {
        return 0;
    }
    if (is_type) {
        return 0;
    }

    value.change_status_type = getChangeStatusType(text);
    if (ScanStr(&text) != 1) {
        return 0;
    }

    value.is_percent = (text == "%");
    bool success = false;
    value.value = static_cast<float>(ScanInt(&success));
    return success;
}

// IDA_EA: 0x88BDB27
// IDA_NAME: _Z16scanVariableStatR12VariableStat
// DEMANGLED: scanVariableStat(VariableStat &)
int scanVariableStat(VariableStat& value) {
    if (scanChangeStatusData(value.status)) {
        return scanScriptValue(value.script_value);
    }
    return 0;
}

typedef char ChangeStatusData_Size_Must_Be_0x0C[
    (sizeof(ChangeStatusData) == 0x0C) ? 1 : -1];
typedef char VariableStat_Size_Must_Be_0x20[
    (sizeof(VariableStat) == 0x20) ? 1 : -1];
