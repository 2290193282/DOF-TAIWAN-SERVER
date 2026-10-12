#ifndef OLDSRV_OLD_GAME_PVP_ARENA_ZOMBIE_OWNER_SNAPSHOT_H_
#define OLDSRV_OLD_GAME_PVP_ARENA_ZOMBIE_OWNER_SNAPSHOT_H_

#include <stddef.h>

class CUser;
class PvP_Room;

// Layout-neutral Type19 roster bridges.  The member array is the compact
// logical combat-seat order; its entries may occupy arbitrary physical
// PvP_Room seats after a loading-time 6 -> 5 downgrade.  Every function
// validates that each member is unique and still belongs to the room before
// emitting a packet.
bool SendZombieOwnerFullActorSnapshot(PvP_Room* room,
                                      CUser* recipient,
                                      CUser* const* members,
                                      size_t member_count);

bool SendZombieOwnerCombatSeatMapping(PvP_Room* room,
                                      CUser* recipient,
                                      CUser* const* members,
                                      size_t member_count,
                                      int recipient_party_index);

bool SendZombieOwnerFullPeerSnapshot(PvP_Room* room,
                                     CUser* recipient,
                                     CUser* const* members,
                                     size_t member_count);

// Compatibility composition retained for the current authority until its
// packet ordering is migrated.  It deliberately preserves the old
// NOTI2-then-NOTI9 behavior and does not add packet 11 implicitly.
bool SendZombieOwnerSnapshot(PvP_Room* room,
                             CUser* recipient,
                             CUser* const* members,
                             size_t member_count,
                             int recipient_party_index);

#endif  // OLDSRV_OLD_GAME_PVP_ARENA_ZOMBIE_OWNER_SNAPSHOT_H_
