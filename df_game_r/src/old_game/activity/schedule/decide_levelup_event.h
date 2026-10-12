#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_DECIDE_LEVELUP_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_DECIDE_LEVELUP_EVENT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;
struct SIG_MAILBOX_PERSISTENCE_CONTEXT;

namespace decide_levelup_event {

// Account-login state restore and event-character lifecycle hooks.  The
// reservation is account-scoped until CMD5 creates and binds its character.
void RequestLoginState(CUser* user);
void ResetSession(CUser* user);
bool HasPendingReservation(CUser* user);
void BindCreatedCharacter(CUser* user,
                          unsigned int charac_no,
                          int event_character_grow_type);
void OnLevelUp(CUser* user);
void OnTrackedMailResult(const SIG_MAILBOX_PERSISTENCE_CONTEXT& context,
                         bool persisted);

}  // namespace decide_levelup_event

class Dispatcher_DecideLevelUp
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_DecideLevelUp();
    virtual ~Dispatcher_DecideLevelUp();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_DecideLevelUp : public ISigDispatcher {
public:
    Inter_DecideLevelUp();
    virtual ~Inter_DecideLevelUp();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DecideLevelUp) == 0x04u,
              "CMD561 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_DecideLevelUp) == 0x04u,
              "CMD561 DB reply dispatcher must retain base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_DECIDE_LEVELUP_EVENT_H_
