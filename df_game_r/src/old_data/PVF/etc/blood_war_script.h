#ifndef OLDSRV_OLD_DATA_PVF_ETC_BLOOD_WAR_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_BLOOD_WAR_SCRIPT_H_

#include <string>
#include <utility>
#include <vector>

// IDA unit: RDARScriptEtcParameter.  The two table classes are aggregate
// wrappers: their vector is at offset zero and is passed directly to the
// original vector helpers.
struct stBloodWarGrowTable {
    std::vector<std::vector<std::pair<int, int> > > rows;
};

struct stBloodWarBgmTable {
    stBloodWarBgmTable();
    ~stBloodWarBgmTable();
    stBloodWarBgmTable* clear();

    int map_index;
    std::string filename;
};

struct stBloodPoint {
    stBloodPoint();
    stBloodPoint* clear();

    int type;
    int first_value;
    int second_value;
    int third_value;
};

struct stBloodWarBloodPointTable {
    std::vector<stBloodPoint> points;
};

struct stUltimateWarMonsterInfo {
    stUltimateWarMonsterInfo();
    stUltimateWarMonsterInfo* clear();

    int monster_index;
    int first_value;
    int second_value;
};

// IDA_EA: 0x890E3F8
// IDA_NAME: _Z23importBloodWarEtcScriptPKcP19stBloodWarGrowTableP25stBloodWarBloodPointTablePSt6vectorI18stBloodWarBgmTableSaIS6_EEPS5_I24stUltimateWarMonsterInfoSaISA_EE
// DEMANGLED: importBloodWarEtcScript(char const*,stBloodWarGrowTable*,stBloodWarBloodPointTable*,std::vector<stBloodWarBgmTable>*,std::vector<stUltimateWarMonsterInfo>*)
int importBloodWarEtcScript(
    const char* filename,
    stBloodWarGrowTable* grow_table,
    stBloodWarBloodPointTable* blood_points,
    std::vector<stBloodWarBgmTable>* bgm_table,
    std::vector<stUltimateWarMonsterInfo>* ultimate_monsters);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stBloodWarGrowTable) == 0x0Cu,
              "stBloodWarGrowTable must retain the original vector ABI");
static_assert(sizeof(stBloodWarBgmTable) == 0x08u,
              "stBloodWarBgmTable must retain the original value ABI");
static_assert(sizeof(stBloodPoint) == 0x10u,
              "stBloodPoint must retain the original value ABI");
static_assert(sizeof(stBloodWarBloodPointTable) == 0x0Cu,
              "stBloodWarBloodPointTable must retain the original vector ABI");
static_assert(sizeof(stUltimateWarMonsterInfo) == 0x0Cu,
              "stUltimateWarMonsterInfo must retain the original value ABI");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_ETC_BLOOD_WAR_SCRIPT_H_
