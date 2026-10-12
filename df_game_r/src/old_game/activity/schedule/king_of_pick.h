#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KING_OF_PICK_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KING_OF_PICK_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/activity/reward/event_base.h"

class CUser;
class PacketBuf;

class CKingOfPickEvent : public CEventBase {
public:
    CKingOfPickEvent();
    virtual ~CKingOfPickEvent();

    virtual void StartEvent();
    virtual void StartEvent(Word_Param param);
    virtual void EndEvent();
};

namespace king_of_pick {

void RequestLoginState(CUser* user, bool seed_proper_watermark = false);
// A real false->true transition invalidates snapshots retained from the
// previous run and reloads authoritative day/week state exactly once.
void OnEventStarted(CUser* user);
// The startup-installed minute timer retries a failed/missing active load
// without resending already-loaded CMD589 snapshots every minute.
void OnGlobalMinuteTick();
void ResetSession(CUser* user);
// Invalidates every in-memory source generation before the daily world loop.
void OnProperClearGlobalReset();
// Called immediately after the authoritative daily source counter is cleared.
// This prevents an old-day non-reward boundary from excluding the first
// eligible clear in the new serial generation.
void OnProperClearSerialReset(CUser* user);

// Wire this immediately after the native IncProperDungeonClearCount() in the
// existing common-progress/proper-dungeon branch.  The persisted source
// counter is the replay watermark; this function never changes CParty size.
void OnProperDungeonClear(CUser* user);

}  // namespace king_of_pick_0828

class Dispatcher_LetsPickPresent
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_LetsPickPresent();
    virtual ~Dispatcher_LetsPickPresent();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestAddPickChance
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_RequestAddPickChance();
    virtual ~Dispatcher_RequestAddPickChance();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_KingOfPick : public ISigDispatcher {
public:
    Inter_KingOfPick();
    virtual ~Inter_KingOfPick();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(CKingOfPickEvent) == 0x0Cu,
              "King-of-Pick repeat event must remain base-only");
static_assert(sizeof(Dispatcher_LetsPickPresent) == 0x04u,
              "CMD590 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_RequestAddPickChance) == 0x04u,
              "CMD591 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_KingOfPick) == 0x04u,
              "King-of-Pick DB reply dispatcher must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KING_OF_PICK_0828_H_
