#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KIRI_CARGO_PACKET_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KIRI_CARGO_PACKET_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace KiriCargo {

// REMOTE_DB_EVIDENCE: taiwan_game_event.event_ingame_history already owns
// account-scoped (type, history_1, history_2) rows.  The target 0828
// Event/ingameeventlist.evt occupies record types 1 and 3..14, while the
// restored Rainbow Snake owner reserves 15.  Type 16 is therefore the next
// isolated native record for Kiri Cargo in this tree:
//   history_1 = cargo expiry epoch seconds (zero means no active cargo)
//   history_2 = 1 while the cargo is active, otherwise zero.
enum {
    kInGameEventHistoryType = 16,
};

// Native NOTI602 producer.  The login completion owner must call this after
// event_ingame_history has been imported; CMD542/543 also call it after each
// successful transition.
void SendInfo(CUser* user);

}  // namespace KiriCargo

class Dispatcher_KiriCargoPurchase
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_KiriCargoPurchase();
    virtual ~Dispatcher_KiriCargoPurchase();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_KiriCargoGetBonus
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_KiriCargoGetBonus();
    virtual ~Dispatcher_KiriCargoGetBonus();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_KiriCargoPurchase) == 0x04u,
              "CMD542 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_KiriCargoGetBonus) == 0x04u,
              "CMD543 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_KIRI_CARGO_PACKET_DISPATCHERS_H_
