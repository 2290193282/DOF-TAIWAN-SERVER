#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_REFERENCE_TABLES_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_REFERENCE_TABLES_H_

#include <vector>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA_EA: ctor @0x837F544, dtor @0x837F558.
// The old i386 value consists solely of one std::vector<unsigned int>.
struct stSpPerLevelTable {
    stSpPerLevelTable();
    ~stSpPerLevelTable();

    std::vector<unsigned int> values;
};

// IDA_EA: 0x890857B
// IDA_NAME: _Z23importPvPReferenceTablePKcR15stPvpRefTable_t
// DEMANGLED: importPvPReferenceTable(char const*,stPvpRefTable_t &)
int importPvPReferenceTable(const char* filename, stPvpRefTable_t& table);

// IDA_EA: 0x8910505
// IDA_NAME: _Z30importSpPerLevelReferenceTablePKcR17stSpPerLevelTable
// DEMANGLED: importSpPerLevelReferenceTable(char const*,stSpPerLevelTable &)
int importSpPerLevelReferenceTable(const char* filename,
                                   stSpPerLevelTable& table);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stSpPerLevelTable) == 0x0Cu,
              "stSpPerLevelTable IDA ABI size must stay 0x0C");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_REFERENCE_TABLES_H_
