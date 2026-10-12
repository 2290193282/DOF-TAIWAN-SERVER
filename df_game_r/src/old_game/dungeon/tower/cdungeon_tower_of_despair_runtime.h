#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_CDUNGEON_TOWER_OF_DESPAIR_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_CDUNGEON_TOWER_OF_DESPAIR_RUNTIME_H_

#include "old_data/PVF/map/datamanager/map_runtime.h"

class CUser;

// IDA unit: 09_dungeon_pvp_party/CDungeon_TowerOfDespair.
// The derived object changes only CDungeon's sole virtual slot.  It adds no
// members and therefore always uses the base object's allocation extent.
class CDungeon_TowerOfDespair : public CDungeon {
public:
    // IDA_EA: 0x8377606
    // IDA_NAME: _ZN23CDungeon_TowerOfDespairC2Ev
    // DEMANGLED: CDungeon_TowerOfDespair::CDungeon_TowerOfDespair(void)
    CDungeon_TowerOfDespair();

    // IDA_EA: 0x834D8EC
    // IDA_NAME: _ZNK23CDungeon_TowerOfDespair23_do_after_dungeon_startEP5CUser
    // DEMANGLED: CDungeon_TowerOfDespair::_do_after_dungeon_start(CUser *)const
    virtual int _do_after_dungeon_start(CUser* user) const;
};

static_assert(sizeof(CDungeon_TowerOfDespair) == sizeof(CDungeon),
              "Tower of Despair dungeon must retain the base allocation ABI");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_CDUNGEON_TOWER_OF_DESPAIR_RUNTIME_H_
