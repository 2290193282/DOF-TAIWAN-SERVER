#ifndef OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_H_
#define OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/pvp/tournament/pvp_tournament_types.h"

class CUser;
class PacketBuf;
class PvP_Room;

namespace pvp_tournament {

// First-town-ready/session-generation hooks.  The feature fails closed until
// Event/tw_pvp_tournament_0828.evt has parsed and every official constant
// matches SQL021.
void BeginSession(CUser* user);
void ResetSession(CUser* user);

// Call from the dispatch-thread periodic owner (one call per second is fine).
// It renews no client state; every 60 seconds it queues one userless expiry
// sweep and also releases stale in-memory room bindings.
void OnPeriodicTick(uint32_t now_epoch);

// Room-pool cleanup integration.  Safe to call more than once.  This restores
// the legacy tournament IMatch pointer before PvP_Room returns to its pool.
void OnRoomReturned(PvP_Room* room);

bool IsPolicyLoaded();

}  // namespace pvp_tournament_0828

class Dispatcher_PvpTournamentMatchList
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PvpTournamentMatchList();
    virtual ~Dispatcher_PvpTournamentMatchList();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_PvpTournamentRequest
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PvpTournamentRequest();
    virtual ~Dispatcher_PvpTournamentRequest();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterPvpTournament : public TaiwanInterfaceInterDispatcher {
public:
    InterPvpTournament();
    virtual ~InterPvpTournament();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PvpTournamentMatchList) == 0x04u,
              "CMD544 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_PvpTournamentRequest) == 0x04u,
              "CMD545 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterPvpTournament) == 0x08u,
              "tournament Taiwan inter owner must preserve base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_0828_H_
