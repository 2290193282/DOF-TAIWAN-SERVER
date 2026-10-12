#ifndef OLDSRV_DATAMANAGER_SCRIPT_VALUE_H_
#define OLDSRV_DATAMANAGER_SCRIPT_VALUE_H_

#include <stdint.h>

#include <string>
#include <vector>


// IDA_EA: 0x88C4200, 0x83661E2, 0x8366220, 0x88C29A2, 0x88C2C34, 0x88BD381, 0x88BD8E6, 0x88BD929, 0x88BD984.
// IDA unit: 12_pvf_datamanager_scripts/ScriptValue. Old i386 size: 0x14.
struct ScriptValue {
    ScriptValue();
    ScriptValue(const ScriptValue& other);

    ScriptValue& operator=(const ScriptValue& other);

    int32_t value_type;  // +0x00, default 3 in old constructor.
    int32_t values[4];   // +0x04..+0x13
};

int scanScriptValue(ScriptValue& value);
int scanScriptValue(std::vector<float>& values);
bool pushScriptValue(std::vector<float>& values, ScriptValue& value);
bool getScriptValue(ScriptValue& value, std::vector<float>& values, int index);
int getActiveStatusToEnumValue(std::string& value);
int getTargetToEnumValue(std::string& value);

// IDA_EA: 0x89BBE1E.
// IDA_NAME: _Z32getActivePropertyTypeToEnumValueRSs
// DEMANGLED: getActivePropertyTypeToEnumValue(std::string &)
int getActivePropertyTypeToEnumValue(std::string& value);

// IDA_EA: 0x89BCB06 / 0x89BCDE0.
// IDA_NAME: _Z18getBuffToEnumValueRSs / _Z30getExcepttionStatusToEnumValueRSs
int getBuffToEnumValue(std::string& value);
int getExcepttionStatusToEnumValue(std::string& value);

#endif  // OLDSRV_DATAMANAGER_SCRIPT_VALUE_H_
