#include "old_game/world/region/world_area_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/battlefield/pvp_assault.h"

#include <limits.h>
#include <stddef.h>

namespace pvp_assault {

// IDA_EA: 0x82E5A0B
// IDA_NAME: _ZN11pvp_assault25IsIncludeVirtualMoveAreasEPSt6vectorI7MapAreaSaIS1_EEiiRiS5_i
// DEMANGLED: pvp_assault::IsIncludeVirtualMoveAreas(std::vector<MapArea,std::allocator<MapArea>> *,int,int,int &,int &,int)
int IsIncludeVirtualMoveAreas(std::vector<MapArea>* map_areas,
                              int pos_x,
                              int pos_y,
                              int& moved_x,
                              int& moved_y,
                              int margin) {
    unsigned int nearest_distance = INT_MAX;
    int clamped_x = 0;
    int clamped_y = 0;

    moved_x = pos_x;
    moved_y = pos_y;
    for (size_t index = 0; index < map_areas->size(); ++index) {
        MapArea& area = map_areas->at(index);
        if (area.include(pos_x,
                         pos_y,
                         clamped_x,
                         clamped_y,
                         margin)) {
            return 1;
        }

        const unsigned int distance = static_cast<unsigned int>(
            GetDistance(Point2d(pos_x, pos_y),
                        Point2d(clamped_x, clamped_y)));
        if (nearest_distance > distance) {
            if (margin != 0) {
                moved_x = clamped_x;
                moved_y = clamped_y;
            } else {
                moved_x = clamped_x - pos_x;
                moved_y = clamped_y - pos_y;
            }
            nearest_distance = distance;
        }
    }
    return 0;
}

}  // namespace pvp_assault

// IDA_EA: 0x82E59F1
// IDA_NAME: _Z11GetDistanceN11pvp_assault7Point2dES0_
// DEMANGLED: GetDistance(pvp_assault::Point2d,pvp_assault::Point2d)
int GetDistance(pvp_assault::Point2d lhs, pvp_assault::Point2d rhs) {
    return static_cast<int>(lhs - rhs);
}

// IDA_EA: 0x82E5ED1
// IDA_NAME: _Z25IsIncludeVirtualMoveAreasPSt6vectorI7MapAreaSaIS0_EEii
// DEMANGLED: IsIncludeVirtualMoveAreas(std::vector<MapArea,std::allocator<MapArea>> *,int,int)
int IsIncludeVirtualMoveAreas(std::vector<MapArea>* map_areas,
                              int pos_x,
                              int pos_y) {
    for (size_t index = 0; index < map_areas->size(); ++index) {
        if (map_areas->at(index).include(pos_x, pos_y) != 1) {
            return 0;
        }
    }
    return 1;
}
