#ifndef OLDSRV_OLD_GAME_DUNGEON_MISSION_DUNGEON_MISSION_H_
#define OLDSRV_OLD_GAME_DUNGEON_MISSION_DUNGEON_MISSION_H_

class CBattle_Field;
class CParty;
class CUser;
class InterfacePacketBuf;
class PacketBuf;

namespace dungeon_mission {

// Appends the 140828 mission tail that follows the ordinary NOTI28 dungeon
// fields.  A disabled/unavailable mission is encoded as descriptor -1.
void WriteDungeonInfoTail(CParty* party,
                          CBattle_Field* battle_field,
                          InterfacePacketBuf* packet);

// Returns the per-room PVF resource consumed by the 0828 NOTI29/START_MAP
// reader.  The client creates the mission-local role from this ID; -1 means
// that the current room has no mission role.
int GetStartMapResourceIndex(CParty* party, CBattle_Field* battle_field);

void ResetParty(CParty* party);

// Proven NOTI609 action-0 writer for the hidden-monster primary/dummy layout.
// Other mission types use a different vector partition and fail closed here.
bool SendTimerLayout(CUser* user);

// Native CMD552/553/554 feature handlers used by the vptr-only dispatchers.
int HandleStart(CUser* user, PacketBuf& packet);
int HandleUpdate(CUser* user, PacketBuf& packet);
int HandleCheckSuccess(CUser* user, PacketBuf& packet);

}  // namespace dungeon_mission

#endif  // OLDSRV_OLD_GAME_DUNGEON_MISSION_DUNGEON_MISSION_H_
