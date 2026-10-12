#ifndef OLDSRV_OLD_DATA_PVF_DATA_CNRD_SCRIPT_DATA_H_
#define OLDSRV_OLD_DATA_PVF_DATA_CNRD_SCRIPT_DATA_H_

#include "old_data/PVF/common/legacy_string_abi.h"

// IDA_EA: 0x807F858, 0x807F966, 0x807FA58, 0x807FABC, 0x807FAC2,
// 0x807FAC8, 0x807FB2C, 0x807FE8A, 0x807FEBC, 0x807FEF4, 0x807FF30,
// 0x807FF62.
// IDA unit: 12_pvf_datamanager_scripts/CNRDScriptData.
class CNRDScriptData {
public:
    CNRDScriptData();
    ~CNRDScriptData();

    int clear();
    void create();
    void destroy();
    int reset();
    int import(const char* filename);
    int getInt(const char* key) const;
    long double getFloat(const char* key) const;
    const char* getString(const char* key) const;
    int getCLong(const char* key) const;
    long double getCFloat(const char* key) const;

private:
    CharString filename_;
    StringUnorderedMap<CharStringHashTrait, int> int_values_;
    StringUnorderedMap<CharStringHashTrait, float> float_values_;
    StringUnorderedMap<CharStringHashTrait, int> clong_values_;
    StringUnorderedMap<CharStringHashTrait, float> cfloat_values_;
    StringUnorderedMap<CharStringHashTrait, std::string> string_values_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CNRDScriptData) == 0xA4u,
              "CNRDScriptData IDA ABI size must stay 0xA4");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_DATA_CNRD_SCRIPT_DATA_H_
