#include "old_data/PVF/map/datamanager/map_runtime.h"

// IDA_EA: 0x8891C18
// IDA_NAME: _Z12makePathAreaP7MapAreaPA2_KiS3_
// DEMANGLED: makePathArea(MapArea *,int const(*)[2],int const(*)[2])
bool makePathArea(MapArea* areas,
                  const int (*positions)[2],
                  const int (*sizes)[2]) {
    for (int index = 0; index <= 3; ++index) {
        MapArea& area = areas[index];

        if (index > 1) {
            area.width = sizes[index][0] <= 0 ? 140 : sizes[index][0];
            area.height = sizes[index][1] <= 0 ? 40 : sizes[index][1];
            area.x = positions[index][0] - area.width / 2;
        } else {
            area.width = sizes[index][0] <= 0 ? 120 : sizes[index][0];
            area.height = sizes[index][1] <= 0 ? 106 : sizes[index][1];
            area.x = index == 0
                ? positions[index][0]
                : positions[index][0] - area.width;
        }
        area.y = positions[index][1] - area.height / 2;
    }

    // The original loop returns its final n3 <= 3 comparison, which is false.
    return false;
}

// IDA_EA: 0x889203B
// IDA_NAME: _Z8isInAreaiiP7MapArea
// DEMANGLED: isInArea(int,int,MapArea *)
bool isInArea(int pos_x, int pos_y, MapArea* area) {
    if (area->x > pos_x) {
        return false;
    }
    if (area->x + area->width <= pos_x) {
        return false;
    }
    if (area->y <= pos_y) {
        return area->y + area->height > pos_y;
    }
    return false;
}

// IDA_EA: 0x8891F20
// IDA_NAME: _Z12isInPathAreaiiPA2_KiS1_
// DEMANGLED: isInPathArea(int,int,int const(*)[2],int const(*)[2])
int isInPathArea(int pos_x,
                 int pos_y,
                 const int (*positions)[2],
                 const int (*sizes)[2]) {
    MapArea areas[4];
    makePathArea(areas, positions, sizes);

    for (int index = 0; index <= 3; ++index) {
        if (isInArea(pos_x, pos_y, &areas[index])) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x834E9A6
// IDA_NAME: _ZNK4CMap20checkFitInPathObjectEii
// DEMANGLED: CMap::checkFitInPathObject(int,int)const
int CMap::checkFitInPathObject(int pos_x, int pos_y) const {
    return isInPathArea(pos_x,
                        pos_y,
                        pathgate_positions_,
                        pathgate_recognize_ranges_);
}
