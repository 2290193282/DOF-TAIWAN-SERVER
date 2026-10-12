#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_LEVELUP_SUPPORT_3RD_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_LEVELUP_SUPPORT_3RD_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

namespace levelup_support_3rd {

// Character-session hooks.  The login owner loads the persistent watermark;
// level-up sends the client's two-byte NOTI660 state, and pooled-user reset
// invalidates stale DB replies.
void RequestLoginState(CUser* user);
void ResetSession(CUser* user);
void OnLevelUp(CUser* user);

}  // namespace levelup_support_3rd_0828

class Dispatcher_LevelupSupport3rdEventGetItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_LevelupSupport3rdEventGetItem();
    virtual ~Dispatcher_LevelupSupport3rdEventGetItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_LevelupSupport3rd : public ISigDispatcher {
public:
    Inter_LevelupSupport3rd();
    virtual ~Inter_LevelupSupport3rd();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_LevelupSupport3rdEventGetItem) == 0x04u,
              "CMD712 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_LevelupSupport3rd) == 0x04u,
              "CMD712 DB reply dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_LEVELUP_SUPPORT_3RD_0828_H_

