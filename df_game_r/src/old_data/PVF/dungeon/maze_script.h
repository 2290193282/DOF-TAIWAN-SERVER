#ifndef OLDSRV_DATAMANAGER_MAZE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_MAZE_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

#include "old_data/PVF/dungeon/grid_script.h"

// PVF map/dungeon clear-condition record.  It is parsed with the maze data;
// gameplay consumers only receive the typed value after DataManager loading.
// IDA_EA: 0x88F29D2, 0x88F29F4, 0x88F7CD0.
struct DungeonClearCondition {
    DungeonClearCondition();
    ~DungeonClearCondition();

    DungeonClearCondition* clear();

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
};

// IDA_EA: 0x830E468, 0x830E47C, 0x8312844.
struct RidableObject {
    RidableObject();
    ~RidableObject();

    RidableObject* clear();

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
};

// IDA_EA: 0x836A18A, 0x8376D52, 0x83A47F8, 0x88F2A08, 0x88F2A50.
struct RidableScript {
    RidableScript();
    RidableScript(const RidableScript& rhs);
    ~RidableScript();

    RidableScript& operator=(const RidableScript& rhs);
    RidableScript* clear();

    int32_t value_00;
    uint8_t value_04;
    std::vector<RidableObject> objects;
};

// IDA_EA: 0x822ADC2, 0x8376D8A, 0x8376F00, 0x83A4832,
// 0x88F2A76, 0x88F2B4A, 0x88F2C2A.
struct MazeScript {
    MazeScript();
    MazeScript(const MazeScript& rhs);
    ~MazeScript();

    MazeScript& operator=(const MazeScript& rhs);
    MazeScript* clear();
    GridScript& getGrid(int x, int y);
    const GridScript& getGridR(int x, int y) const;

    int32_t grid_width;
    int32_t grid_height;
    std::vector<GridScript> grids;
    std::vector<std::pair<int, int> > values_14;
    std::vector<std::pair<int, int> > values_20;
    int32_t value_2c;
    int32_t value_30;
    int32_t value_34;
    RidableScript ridable;
    int32_t value_4c;
    int32_t value_50;
    int32_t value_54;
    std::vector<DungeonClearCondition> clear_conditions;
    int32_t maze_index;
    int32_t value_68;
    int32_t value_6c;
    uint8_t value_70;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DungeonClearCondition) == 0x0c,
              "DungeonClearCondition IDA ABI size must stay 0x0C");
static_assert(sizeof(RidableObject) == 0x18,
              "RidableObject IDA ABI size must stay 0x18");
static_assert(sizeof(RidableScript) == 0x14,
              "RidableScript IDA ABI size must stay 0x14");
static_assert(sizeof(MazeScript) == 0x74,
              "MazeScript IDA ABI size must stay 0x74");
static_assert(offsetof(MazeScript, maze_index) == 0x64,
              "MazeScript maze index must stay at +0x64");
#endif

#endif  // OLDSRV_DATAMANAGER_MAZE_SCRIPT_H_
