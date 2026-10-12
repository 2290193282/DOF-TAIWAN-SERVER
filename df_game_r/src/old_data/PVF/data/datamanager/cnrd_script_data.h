#ifndef OLDSRV_DATAMANAGER_CNRD_SCRIPT_DATA_H_
#define OLDSRV_DATAMANAGER_CNRD_SCRIPT_DATA_H_

#include <map>
#include <string>


// IDA_EA: 0x807F858, 0x807F966, 0x807FA58, 0x807FABC, 0x807FAC2, 0x807FAC8, 0x807FB2C, 0x807FE8A, 0x807FEBC, 0x807FEF4, 0x807FF30, 0x807FF62.
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
    typedef std::map<std::string, int> IntMap;
    typedef std::map<std::string, float> FloatMap;
    typedef std::map<std::string, std::string> StringMap;

    std::string filename_;
    IntMap int_values_;
    FloatMap float_values_;
    IntMap clong_values_;
    FloatMap cfloat_values_;
    StringMap string_values_;
};

#endif  // OLDSRV_DATAMANAGER_CNRD_SCRIPT_DATA_H_
