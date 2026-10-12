#ifndef OLDSRV_OLD_GAME_PVP_BATTLEFIELD_ZOMBIE_PREFERRED_START_RUNTIME_H_
#define OLDSRV_OLD_GAME_PVP_BATTLEFIELD_ZOMBIE_PREFERRED_START_RUNTIME_H_

class CBattle_Field;

namespace zombie_preferred_start_runtime {

// Layout-neutral one-shot handoff used by the Zombie matching owner.  The
// caller registers a PVF start coordinate before CParty::dungeon_start; the
// existing CBattle_Field::SetGridPath implementation consumes it only when
// that coordinate is present in the selected MazeScript::values_14 list.
bool Register(CBattle_Field* battle_field, int pos_x, int pos_y);
void Cancel(CBattle_Field* battle_field);

}  // namespace zombie_preferred_start_runtime

#endif  // OLDSRV_OLD_GAME_PVP_BATTLEFIELD_ZOMBIE_PREFERRED_START_RUNTIME_H_
