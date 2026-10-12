#include "old_data/PVF/data/cnrd_script_data.h"

#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>

// IDA_EA: 0x807F858
// IDA_NAME: _ZN14CNRDScriptDataC2Ev
// DEMANGLED: CNRDScriptData::CNRDScriptData(void)
CNRDScriptData::CNRDScriptData()
    : filename_(),
      int_values_(),
      float_values_(),
      clong_values_(),
      cfloat_values_(),
      string_values_() {
    clear();
    create();
}

// IDA_EA: 0x807F966
// IDA_NAME: _ZN14CNRDScriptDataD2Ev
// DEMANGLED: CNRDScriptData::~CNRDScriptData()
CNRDScriptData::~CNRDScriptData() {
    destroy();
}

// IDA_EA: 0x807FA58
// IDA_NAME: _ZN14CNRDScriptData5clearEv
// DEMANGLED: CNRDScriptData::clear(void)
int CNRDScriptData::clear() {
    filename_ = "";
    int_values_.clear();
    float_values_.clear();
    clong_values_.clear();
    cfloat_values_.clear();
    return string_values_.clear();
}

// IDA_EA: 0x807FABC
// IDA_NAME: _ZN14CNRDScriptData6createEv
// DEMANGLED: CNRDScriptData::create(void)
void CNRDScriptData::create() {}

// IDA_EA: 0x807FAC2
// IDA_NAME: _ZN14CNRDScriptData7destroyEv
// DEMANGLED: CNRDScriptData::destroy(void)
void CNRDScriptData::destroy() {}

// IDA_EA: 0x807FAC8
// IDA_NAME: _ZN14CNRDScriptData5resetEv
// DEMANGLED: CNRDScriptData::reset(void)
int CNRDScriptData::reset() {
    filename_ = "";
    int_values_.clear();
    float_values_.clear();
    clong_values_.clear();
    cfloat_values_.clear();
    return string_values_.clear();
}

// IDA_EA: 0x807FB2C
// IDA_NAME: _ZN14CNRDScriptData6importEPKc
// DEMANGLED: CNRDScriptData::import(char const*)
int CNRDScriptData::import(const char* filename) {
    if (loadRDARScriptFile("Data/", filename) != 1) {
        LogManager::logFormat(0,
                              "../../DNFShared/CNRDScriptData.cpp",
                              "import",
                              0x33,
                              "Error!! Script Data import fail. %s",
                              filename);
        return 0;
    }

    reset();
    filename_ = filename;

    std::string tag;
    std::string key;
    std::string value;
    while (ScanType(tag, true) == 1) {
        if (tag == "[int]") {
            while (ScanStr(&key)) {
                const int scanned = ScanInt(static_cast<bool*>(0));
                int_values_.insert(key.c_str(), scanned);
            }
        } else if (tag == "[float]") {
            while (ScanStr(&key)) {
                const float scanned = ScanFloat(static_cast<bool*>(0));
                float_values_.insert(key.c_str(), scanned);
            }
        } else if (tag == "[clong]") {
            while (ScanStr(&key)) {
                const int scanned = ScanInt(static_cast<bool*>(0));
                clong_values_.insert(key.c_str(), scanned);
            }
        } else if (tag == "[cfloat]") {
            while (ScanStr(&key)) {
                const float scanned = ScanFloat(static_cast<bool*>(0));
                cfloat_values_.insert(key.c_str(), scanned);
            }
        } else if (tag == "[string]") {
            while (ScanStr(&key)) {
                ScanStr(&value);
                string_values_.insert(key.c_str(), value);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x807FE8A
// IDA_NAME: _ZNK14CNRDScriptData6getIntEPKc
// DEMANGLED: CNRDScriptData::getInt(char const*)const
int CNRDScriptData::getInt(const char* key) const {
    const int* value = int_values_.get(key);
    return value != 0 ? *value : -1;
}

// IDA_EA: 0x807FEBC
// IDA_NAME: _ZNK14CNRDScriptData8getFloatEPKc
// DEMANGLED: CNRDScriptData::getFloat(char const*)const
long double CNRDScriptData::getFloat(const char* key) const {
    const float* value = float_values_.get(key);
    return value != 0 ? *value : -1.0;
}

// IDA_EA: 0x807FEF4
// IDA_NAME: _ZNK14CNRDScriptData9getStringEPKc
// DEMANGLED: CNRDScriptData::getString(char const*)const
const char* CNRDScriptData::getString(const char* key) const {
    const std::string* value = string_values_.get(key);
    return value != 0 ? value->c_str() : "CNRDScriptData::Error";
}

// IDA_EA: 0x807FF30
// IDA_NAME: _ZNK14CNRDScriptData8getCLongEPKc
// DEMANGLED: CNRDScriptData::getCLong(char const*)const
int CNRDScriptData::getCLong(const char* key) const {
    const int* value = clong_values_.get(key);
    return value != 0 ? *value : -1;
}

// IDA_EA: 0x807FF62
// IDA_NAME: _ZNK14CNRDScriptData9getCFloatEPKc
// DEMANGLED: CNRDScriptData::getCFloat(char const*)const
long double CNRDScriptData::getCFloat(const char* key) const {
    const float* value = cfloat_values_.get(key);
    return value != 0 ? *value : -1.0;
}
