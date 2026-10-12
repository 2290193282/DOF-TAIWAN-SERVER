#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FATIGUE_ACCELERATION_RUNTIME_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FATIGUE_ACCELERATION_RUNTIME_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CDungeon;
class CUser;
class PacketBuf;

namespace fatigue_acceleration {

// Imports event/fatigue/eventfatigue.etc and resolves its buff index through
// etc/variousbufflist.etc.  This 0828-only configuration is optional for a
// 0627 PVF; malformed or incomplete data disables CMD556.
bool LoadConfig();

// CUser is pooled.  BeginSession installs a generation token and queues the
// persisted personal switch; ResetSession invalidates every outstanding DB
// reply before the object can represent another account/character.
void BeginSession(CUser* user);
void ResetSession(CUser* user);

// A successful CParty::dungeon_start (including RestartDungeon) is the owner
// of a new one-match generation.  It resets only transient per-run benefit
// state; the persisted town switch remains untouched.
void BeginDungeonRun(CUser* user,
                     const CDungeon* dungeon,
                     int dungeon_index);

// Clears a generation that was provisionally opened before a late legacy
// dungeon-start failure and removes any installed client buff.  It does not
// change the persisted town switch.
void AbortDungeonRun(CUser* user);

// Server-side mirror of the client's strict predicate:
// current consumed fatigue > NOTI487 threshold ? 2 : 1.
int ResolveFatigueCost(CUser* user,
                       const CDungeon* dungeon,
                       int dungeon_index);

// FatigueUp(0) means one point in the 0627 owner, so the cap-zero path must
// suppress that call rather than pass zero through it.  This wrapper owns the
// cap reservation and the actual FatigueUp transaction.
int ConsumeFatigue(CUser* user,
                   const CDungeon* dungeon,
                   int dungeon_index);

// Returns the additive hunting-EXP percentage for the active one-match
// benefit.  The caller keeps the existing fatigue-bonus EXP accounting slot,
// which prevents a second multiplication and preserves NOTI2 breakdown data.
int GetHunterExpBonusRate(CUser* user,
                          const CDungeon* dungeon,
                          int dungeon_index);

void SendDungeonState(CUser* user,
                      const CDungeon* dungeon,
                      int dungeon_index);
void SendTownState(CUser* user);

}  // namespace fatigue_acceleration

class Dispatcher_FatigueAccelerationStateChange
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_FatigueAccelerationStateChange();
    virtual ~Dispatcher_FatigueAccelerationStateChange();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_FatigueAcceleration : public ISigDispatcher {
public:
    Inter_FatigueAcceleration();
    virtual ~Inter_FatigueAcceleration();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_FatigueAccelerationStateChange) == 0x04u,
              "CMD556 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_FatigueAcceleration) == 0x04u,
              "CMD556 inter dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FATIGUE_ACCELERATION_RUNTIME_H_
