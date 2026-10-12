#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_TAKE_MAGIC_LAMP_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_TAKE_MAGIC_LAMP_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

// Native 0828 CMD723 compatibility owner.  The official client sends an
// exactly empty, fire-and-forget request and has no CMD response handler.
class Dispatcher_TakeMagicLamp
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_TakeMagicLamp();
    virtual ~Dispatcher_TakeMagicLamp();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_TakeMagicLamp : public ISigDispatcher {
public:
    Inter_TakeMagicLamp();
    virtual ~Inter_TakeMagicLamp();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_TakeMagicLamp) == 0x04u,
              "CMD723 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Inter_TakeMagicLamp) == 0x04u,
              "CMD723 inter dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_TAKE_MAGIC_LAMP_0828_H_
