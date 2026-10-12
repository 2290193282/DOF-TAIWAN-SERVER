#ifndef OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_H_
#define OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CParty;
class CUser;
class PacketBuf;

namespace racing_dungeon {

// Dispatch-thread lifecycle boundaries.  All are idempotent and return/no-op
// for ordinary parties and users.
void OnPeriodicTick(uint32_t now_epoch);
void OnUserReset(CUser* user);
void OnPartyMemberLeaving(CParty* party, CUser* user);
void OnPartyDestroyed(CParty* party);

// Call at the final no-fail point of CParty::dungeon_start, immediately before
// normal dungeon packets are emitted.  Pass dungeon_start's requested index;
// only client/official-PVF dungeon 3055 is accepted.  Racing fees, invitation
// and PVF fatigue are committed once here.  A false result aborts the start.
bool CommitDungeonStart(CParty* party, int dungeon_index);

// Call from CParty::check_allmember_loading after every current member has
// loaded and immediately before battle_field_.onStartMap().  This is the sole
// authoritative race-clock start boundary.
void OnAllMembersLoaded(CParty* party);

// The ordinary dungeon owner must not charge its normal room fatigue after
// CommitDungeonStart has charged racingdungeon.info [require fatigue].
bool ShouldSkipOrdinaryFatigue(CParty* party);
bool IsRacingParty(CParty* party);

}  // namespace racing_dungeon_0828

#define DECLARE_RACING_DISPATCHER(class_name)                         \
class class_name                                                     \
    : public IPacketDispatcher<MSG_BASE, ParamBase,                  \
                               static_cast<ch_state>(0)> {           \
public:                                                              \
    class_name();                                                    \
    virtual ~class_name();                                           \
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);        \
    virtual int read(PacketBuf& packet, MSG_BASE& message);          \
    virtual int process(CUser* user, MSG_BASE& message,              \
                        ParamBase& param);                            \
}

DECLARE_RACING_DISPATCHER(Dispatcher_RacingDungeonJoin);
DECLARE_RACING_DISPATCHER(Dispatcher_RacingDungeonDisjoin);
DECLARE_RACING_DISPATCHER(Dispatcher_RacingDungeonGoal);
DECLARE_RACING_DISPATCHER(Dispatcher_RacingDungeonReturn);

#undef DECLARE_RACING_DISPATCHER

class InterRacingDungeon : public TaiwanInterfaceInterDispatcher {
public:
    InterRacingDungeon();
    virtual ~InterRacingDungeon();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RacingDungeonJoin) == 0x04u,
              "CMD523 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_RacingDungeonDisjoin) == 0x04u,
              "CMD524 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_RacingDungeonGoal) == 0x04u,
              "CMD525 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_RacingDungeonReturn) == 0x04u,
              "CMD526 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterRacingDungeon) == 0x08u,
              "Racing Taiwan Inter owner must preserve base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_0828_H_
