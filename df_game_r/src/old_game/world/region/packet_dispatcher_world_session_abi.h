#ifndef OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_ABI_H_
#define OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_ABI_H_

#include <stdint.h>

class CUser;
class GameWorld;

namespace pvp_assault {
class CAssaultMgr;
}

// These exact member declarations preserve the old game-service boundary.
// They are declarations only: their providers are recorded link debt, not
// synthetic world, PvP, or tutorial service bodies.
namespace oldsrv_world_session_abi {

// old-server callee EA 0x86C7AF8, _ZN9GameWorld7fishingEP5CUser
// old-server callee EA 0x82EFC54,
// _ZN11pvp_assault11CAssaultMgr11OnAbuseUserEP5CUser
// old-server callee EA 0x86475D8, _ZN5CUser14RewardTutorialEj
// Old-server callee EA 0x86657FC, _ZN5CUser15increase_statusEs.
// CMD032 needs the exact member symbol but the full character-stat mutation
// provider is intentionally kept as explicit link debt.
// Old-server callee EA 0x8668D1A, _ZN5CUser12UseQuestItemEi.
// The old caller zero-extends its signed-short packet field before this call.
}  // namespace oldsrv_world_session_abi

#endif  // OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_ABI_H_
