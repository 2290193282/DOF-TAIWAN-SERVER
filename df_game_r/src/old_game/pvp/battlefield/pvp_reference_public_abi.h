#ifndef OLDSRV_GLOBAL_PVP_REFERENCE_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_PVP_REFERENCE_PUBLIC_ABI_H_

#include "old_game/economy/inventory/instance_rental_system_abi.h"

// IDA data block:
//   PVP_NORMAL_BATTLE_TIME @0x93F4EE0, dword_93F4EE4 @0x93F4EE4,
//   PVP_DM_BATTLE_TIME     @0x93F4EE8, dword_93F4EEC @0x93F4EEC,
//   PVP_RELAY_BATTLE_TIME  @0x93F4EF0, dword_93F4EF4 @0x93F4EF4.
// PvP_Room::time_out @0x85D9B0E indexes the normal/deathmatch arrays with
// the boolean tournament-channel flag. importPvPReferenceTable @0x890857B
// writes the base slot and the tournament aliases separately.
extern int PVP_NORMAL_BATTLE_TIME[2];
extern int PVP_DM_BATTLE_TIME[2];
extern int PVP_RELAY_BATTLE_TIME;
extern int dword_93F4EE4;
extern int dword_93F4EEC;
extern int dword_93F4EF4;

#endif  // OLDSRV_GLOBAL_PVP_REFERENCE_PUBLIC_ABI_H_
