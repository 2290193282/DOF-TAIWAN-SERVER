#ifndef OLDSRV_OLD_DATA_PVF_ETC_TIME_SPIRAL_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_TIME_SPIRAL_SCRIPT_H_

class CDataManager;
struct STAssignSpecialPassiveObj;

namespace time_spiral {

enum {
    kDungeonIndex = 3900,
    kDesignatedDifficulty = 2,
    kTrapPassiveObjectId = 12577,
    kGridWidth = 5,
    kGridHeight = 5,
};

struct RouteSelection {
    RouteSelection();

    int target_x;
    int target_y;
    int result;
    int weight;
};

// Parsing and publication are deliberately split.  The ETC file is read
// while RDAR is available, but publication waits until passive objects,
// dungeons and maps have all been materialized by CDataManager.
bool ImportConfig(const char* filename);
bool FinalizeConfig(CDataManager* data_manager);
bool IsReady();
bool HasNativeClosure();

bool IsDungeon(int dungeon_index);
// CDungeon::set_dungeon publishes the 0828 DGN-owned fixed difficulty after
// the generic dungeon parser has consumed [designate dungeon difficulty].
// FinalizeConfig validates it before this value becomes admission authority.
void RegisterDesignatedDifficulty(int dungeon_index, int difficulty);
int GetDesignatedDifficulty(int dungeon_index);
int GetTrapPassiveObjectId();

// This is the only admission path from an 0828 [special passive object] into
// the 0627 CMap passive-object authority.  It verifies both the map action
// tuple and the complete passive-object script's [trap] flag.
bool IsValidatedTrapSpecialObject(
    const STAssignSpecialPassiveObj& object);

// CMap::set_map reports the owning map only after the full special-object
// action tuple and the referenced passive-object script have both passed the
// validator above.  Final publication uses this separate registry rather
// than mistaking an ordinary passive object with the same numeric id for a
// validated TimeSpiral trap.
void RegisterValidatedTrapMap(int map_index);

bool IsTrapSource(int map_index, int pos_x, int pos_y);
bool FindCoordinateForMap(int map_index, int* pos_x, int* pos_y);
bool GetNormalExit(int source_map_index, int* pos_x, int* pos_y);
bool SelectRouteByRoll(int source_map_index,
                       int roll,
                       RouteSelection* selection);
bool IsTerminalCoordinate(int dungeon_index, int pos_x, int pos_y);

}  // namespace time_spiral

#endif  // OLDSRV_OLD_DATA_PVF_ETC_TIME_SPIRAL_SCRIPT_H_
