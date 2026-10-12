#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ACCOUNT_FATIGUE_STAT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ACCOUNT_FATIGUE_STAT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;
class PacketBuf;

// 0828 CMD710 / ENUM_CMDPACKET_EVENT_ACCOUNT_FATIGUE_STAT.
// The official client sends an empty request and consumes two u32 values on
// success.  This command did not exist in the 0627 server registry, so this is
// an 0828 compatibility owner rather than a claimed 0627 symbol restoration.
class Dispatcher_EventAccountFatigueStat
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_EventAccountFatigueStat();
    virtual ~Dispatcher_EventAccountFatigueStat();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// Internal DB_Q result consumer for compatibility signal 851.  The query
// itself belongs to DB_EventAccountFatigueStat and therefore never touches a
// mutable MySQL handle from DispatchThread.
class Inter_EventAccountFatigueStat : public ISigDispatcher {
public:
    Inter_EventAccountFatigueStat();
    virtual ~Inter_EventAccountFatigueStat();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EventAccountFatigueStat) == 0x04u,
              "CMD710 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_EventAccountFatigueStat) == 0x04u,
              "CMD710 inter dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ACCOUNT_FATIGUE_STAT_H_
