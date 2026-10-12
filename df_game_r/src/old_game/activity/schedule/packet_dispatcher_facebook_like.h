#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_FACEBOOK_LIKE_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_FACEBOOK_LIKE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

#include <stdint.h>

class CUser;
class PacketBuf;
struct SIG_MAILBOX_PERSISTENCE_CONTEXT;

namespace facebook_like {

// Installs the per-character generation and loads SQL018.  The caller must be
// at the first town-ready boundary (state 3), not account-list state 2.
void BeginSession(CUser* user);
void ResetSession(CUser* user);

// Called after CUser::FatigueUp has committed the absolute used-fatigue value.
// Only the first threshold crossing for the current 06:00 cycle queues DB work.
void OnFatigueChanged(CUser* user, uint16_t used_fatigue);

// Inter signal 128 invokes this only for the exact tracked consumer-2 frame.
// It advances/reverts SQL018 after the postal exec(true) result, even when the
// original CUser disconnected (user_idx=-1 in that case).
void OnTrackedMailResult(const SIG_MAILBOX_PERSISTENCE_CONTEXT& context,
                         bool persisted);

}  // namespace facebook_like_0828

class Dispatcher_FacebookLike
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_FacebookLike();
    virtual ~Dispatcher_FacebookLike();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_FacebookLike : public ISigDispatcher {
public:
    Inter_FacebookLike();
    virtual ~Inter_FacebookLike();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_FacebookLike) == 0x04u,
              "CMD725 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Inter_FacebookLike) == 0x04u,
              "CMD725 inter dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_FACEBOOK_LIKE_0828_H_
