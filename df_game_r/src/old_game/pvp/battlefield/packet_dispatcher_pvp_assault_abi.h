#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_ABI_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_ABI_H_

#include <stdint.h>

class CParty;
class CUser;
class PvP_Room;

namespace pvp_assault {
class CAssaultMgr;
}

// Exact old-server members used by the packet surface below.  The canonical
// owner bodies are still link debt; these declarations do not fabricate an
// implementation or a class layout.
namespace oldsrv_pvp_packet_abi {

// old-server callee EA 0x85DDFBE, _ZN8PvP_Room13request_fightEP5CUser
// old-server callee EA 0x82EF27A,
// _ZN11pvp_assault11CAssaultMgr12OnConnectP2PEP5CUserPci
// old-server callee EA 0x82EF570,
// _ZN11pvp_assault11CAssaultMgr10OnChangeHpEP5CUseri
// old-server callee EA 0x82EF320,
// _ZN11pvp_assault11CAssaultMgr24OnConnectP2PAfterAssaultEP5CUserPci
// old-server callee EA 0x85A3B6A, _ZN6CParty18kill_blood_monsterEP5CUseriPt
}  // namespace oldsrv_pvp_packet_abi

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_ABI_H_
