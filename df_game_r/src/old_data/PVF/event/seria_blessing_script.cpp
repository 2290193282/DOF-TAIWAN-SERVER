#include "old_data/PVF/event/seria_blessing_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <algorithm>
#include <string>


typedef char SeriaBlessingValue_Size_Must_Be_0x14[
    (sizeof(stSeriaBlessingScript::Value) == 0x14) ? 1 : -1];

// IDA_EA: 0x822AF22
// IDA_NAME: _ZN21stSeriaBlessingScript14is_possible_lvEi
// DEMANGLED: stSeriaBlessingScript::is_possible_lv(int)
int stSeriaBlessingScript::is_possible_lv(int32_t charac_level) {
    FindOneByLv predicate(charac_level);
    std::map<int, Value>::iterator found =
        std::find_if(values.begin(), values.end(), predicate);
    return found != values.end();
}

// IDA_EA: 0x837CE24
// IDA_NAME: _ZN21stSeriaBlessingScriptC2Ev
// DEMANGLED: stSeriaBlessingScript::stSeriaBlessingScript(void)
stSeriaBlessingScript::stSeriaBlessingScript()
    : values() {}

// IDA_EA: 0x837CE38
// IDA_NAME: _ZN21stSeriaBlessingScriptD2Ev
// DEMANGLED: stSeriaBlessingScript::~stSeriaBlessingScript()
stSeriaBlessingScript::~stSeriaBlessingScript() {}

// IDA_EA: 0x8693AD6
// IDA_NAME: _ZN21stSeriaBlessingScript9get_valueEiRNS_5ValueE
// DEMANGLED: stSeriaBlessingScript::get_value(int,stSeriaBlessingScript::Value &)
int stSeriaBlessingScript::get_value(int32_t charac_level, Value& value) {
    FindOneByLv predicate(charac_level);
    std::map<int, Value>::iterator found =
        std::find_if(values.begin(), values.end(), predicate);
    if (found == values.end()) {
        return 0;
    }
    value = found->second;
    return 1;
}

// IDA_EA: 0x822AEBE
// IDA_NAME: _ZN21stSeriaBlessingScript11FindOneByLvC2Ei
// DEMANGLED: stSeriaBlessingScript::FindOneByLv::FindOneByLv(int)
stSeriaBlessingScript::FindOneByLv::FindOneByLv(int32_t charac_level_value)
    : charac_level(charac_level_value) {}

// IDA_EA: 0x822AECC
// IDA_NAME: _ZNK21stSeriaBlessingScript11FindOneByLvclERKSt4pairIKiNS_5ValueEE
// DEMANGLED: stSeriaBlessingScript::FindOneByLv::operator()(std::pair<int const,stSeriaBlessingScript::Value> const&)const
bool stSeriaBlessingScript::FindOneByLv::operator()(
    const std::pair<const int, Value>& entry) const {
    const Value& value = entry.second;
    if (value.min_level == value.max_level && value.min_level == charac_level) {
        return true;
    }
    return value.min_level <= charac_level && charac_level < value.max_level;
}

// IDA_EA: 0x8693AA0
// IDA_NAME: _ZN21stSeriaBlessingScript5ValueC2Ev
// DEMANGLED: stSeriaBlessingScript::Value::Value(void)
stSeriaBlessingScript::Value::Value()
    : min_level(0), max_level(0), value_08(0), value_0c(0), value_10(0) {}

// IDA_EA: 0x891063B
// IDA_NAME: _Z25ImportSeriaBlessingScriptPKcR21stSeriaBlessingScript
// DEMANGLED: ImportSeriaBlessingScript(char const*,stSeriaBlessingScript &)
int ImportSeriaBlessingScript(const char* filename,
                              stSeriaBlessingScript& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 10868;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        stSeriaBlessingScript::Value value;
        if (tag == "[Seria Blessing]") {
            const int event_group = ScanInt(&success);
            if (!success) {
                return 10885;
            }
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.min_level = ScanInt(&success);
                if (!success) {
                    return 10889;
                }
                value.max_level = ScanInt(&success);
                if (!success) {
                    return 10893;
                }
                value.value_08 = ScanInt(&success);
                if (!success) {
                    return 10897;
                }
                value.value_0c = ScanInt(&success);
                if (!success) {
                    return 10901;
                }
                value.value_10 = ScanInt(&success);
                if (!success) {
                    return 10905;
                }
                // RepeatEvent(93) currently evaluates schedule group zero.
                // Consume later groups without replacing its level table.
                if (event_group == 0) {
                    script.values[key] = value;
                }
            }
        }
    }
    return 0;
}
