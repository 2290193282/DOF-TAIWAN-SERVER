#include "old_data/PVF/etc/blood_war_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>
#include <vector>

// IDA_EA: 0x891A95C
// IDA_NAME: _ZN18stBloodWarBgmTableC2Ev
// DEMANGLED: stBloodWarBgmTable::stBloodWarBgmTable(void)
stBloodWarBgmTable::stBloodWarBgmTable() : map_index(-1), filename() {}

// IDA_EA: 0x891A9A4
// IDA_NAME: _ZN18stBloodWarBgmTable5clearEv
// DEMANGLED: stBloodWarBgmTable::clear(void)
stBloodWarBgmTable* stBloodWarBgmTable::clear() {
    map_index = -1;
    filename.clear();
    return this;
}

// IDA_EA: 0x891C9FC
// IDA_NAME: _ZN18stBloodWarBgmTableD2Ev
// DEMANGLED: stBloodWarBgmTable::~stBloodWarBgmTable(void)
stBloodWarBgmTable::~stBloodWarBgmTable() {}

// IDA_EA: 0x891A9C4
// IDA_NAME: _ZN12stBloodPointC2Ev
// DEMANGLED: stBloodPoint::stBloodPoint(void)
stBloodPoint::stBloodPoint() { clear(); }

// IDA_EA: 0x891A9D8
// IDA_NAME: _ZN12stBloodPoint5clearEv
// DEMANGLED: stBloodPoint::clear(void)
stBloodPoint* stBloodPoint::clear() {
    type = 4;
    first_value = 0;
    second_value = 0;
    third_value = 0;
    return this;
}

// IDA_EA: 0x891AA04
// IDA_NAME: _ZN24stUltimateWarMonsterInfoC2Ev
// DEMANGLED: stUltimateWarMonsterInfo::stUltimateWarMonsterInfo(void)
stUltimateWarMonsterInfo::stUltimateWarMonsterInfo() { clear(); }

// IDA_EA: 0x891AA18
// IDA_NAME: _ZN24stUltimateWarMonsterInfo5clearEv
// DEMANGLED: stUltimateWarMonsterInfo::clear(void)
stUltimateWarMonsterInfo* stUltimateWarMonsterInfo::clear() {
    monster_index = 0;
    first_value = 0;
    second_value = 0;
    return this;
}

// IDA_EA: 0x890E3F8
// IDA_NAME: _Z23importBloodWarEtcScriptPKcP19stBloodWarGrowTableP25stBloodWarBloodPointTablePSt6vectorI18stBloodWarBgmTableSaIS6_EEPS5_I24stUltimateWarMonsterInfoSaISA_EE
// DEMANGLED: importBloodWarEtcScript(char const*,stBloodWarGrowTable*,stBloodWarBloodPointTable*,std::vector<stBloodWarBgmTable>*,std::vector<stUltimateWarMonsterInfo>*)
int importBloodWarEtcScript(
    const char* filename,
    stBloodWarGrowTable* grow_table,
    stBloodWarBloodPointTable* blood_points,
    std::vector<stBloodWarBgmTable>* bgm_table,
    std::vector<stUltimateWarMonsterInfo>* ultimate_monsters) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool scanned = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[BGM]") {
            for (;;) {
                stBloodWarBgmTable entry;
                entry.map_index = ScanInt(&scanned);
                if (!scanned || ScanStr(&text) != 1) {
                    break;
                }
                entry.filename = text;
                if (bgm_table != 0) {
                    bgm_table->push_back(entry);
                }
            }
        } else if (tag == "[BP TABLE]") {
            for (;;) {
                std::vector<std::pair<int, int> > row;
                for (int index = 0; index <= 4; ++index) {
                    const int first = ScanInt(&scanned);
                    if (!scanned) {
                        break;
                    }
                    const int second = ScanInt(&scanned);
                    if (!scanned) {
                        break;
                    }
                    row.push_back(std::make_pair(first, second));
                }
                if (!scanned) {
                    break;
                }
                if (grow_table != 0) {
                    grow_table->rows.push_back(row);
                }
            }
        } else if (tag == "[BLOOD POINT]") {
            while (ScanStr(&text) == 1) {
                int type = 4;
                if (text == "named") {
                    type = 0;
                } else if (text == "crazy") {
                    type = 1;
                } else if (text == "round") {
                    type = 2;
                } else if (text == "nomal") {
                    type = 3;
                }

                const int first = ScanInt(&scanned);
                if (!scanned) {
                    break;
                }
                const int second = ScanInt(&scanned);
                if (!scanned) {
                    break;
                }
                const int third = ScanInt(&scanned);
                if (!scanned) {
                    break;
                }
                if (blood_points != 0 && type != 4) {
                    stBloodPoint point;
                    point.type = type;
                    point.first_value = first;
                    point.second_value = second;
                    point.third_value = third;
                    blood_points->points.push_back(point);
                }
            }
        } else if (tag == "[BLOOD INFO]") {
            stUltimateWarMonsterInfo monster;
            monster.monster_index = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            monster.first_value = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            monster.second_value = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            ultimate_monsters->push_back(monster);
        }
    }
    return 1;
}
