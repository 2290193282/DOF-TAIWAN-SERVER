#ifndef OLDSRV_DATAMANAGER_GRID_SCRIPT_H_
#define OLDSRV_DATAMANAGER_GRID_SCRIPT_H_

#include <stdint.h>
#include <vector>


// IDA unit: 12_pvf_datamanager_scripts/GridScript.
class GridScript {
public:
    GridScript();
    GridScript(const GridScript& rhs);
    ~GridScript();

    GridScript& operator=(const GridScript& rhs);
    void clear();

    int32_t grid_type;
    std::vector<int> values_04;
    std::vector<int> values_10;
    std::vector<int> values_1c;
};

#endif  // OLDSRV_DATAMANAGER_GRID_SCRIPT_H_
