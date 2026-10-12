#ifndef OLDSRV_GLOBAL_GUILD_MONITOR_STATE_DISPATCHERS_A_H_
#define OLDSRV_GLOBAL_GUILD_MONITOR_STATE_DISPATCHERS_A_H_

#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

namespace GuildKeySync {

// A transition notification (Inter 97/98) may replace an older non-zero
// cache entry.  A member-list snapshot (Inter 91/198) is only allowed to
// repair an empty cache or confirm the same key, so a delayed page cannot
// roll a character back to a previous guild.
enum Policy {
    kTransition,
    kSnapshotRepair,
    kConfirmation
};

enum Result {
    kRejected,
    kSynchronized,
    kConflict
};

// Synchronize a non-zero key supplied by the guild/monitor service.  The
// helper owns the selected-character match, login-list projection, in-world
// cache update, native 0828 NOTI 71 projection and USERINFO refresh.  It
// deliberately rejects zero: Inter_MonitorSetGuildKey retains the original
// zero-key secede/reset path.
Result SynchronizeAuthoritativeNonZero(CUser* user,
                                       uint32_t charac_no,
                                       uint32_t guild_key,
                                       Policy policy);

}  // namespace GuildKeySync

// These are base-only ISigDispatcher children in the original i386 runtime.
// The Taiwan inter dispatcher owns their allocation and virtual destruction.
class Inter_MonitorSetGuildKey : public ISigDispatcher {
public:
    Inter_MonitorSetGuildKey();
    virtual ~Inter_MonitorSetGuildKey();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildCreate : public ISigDispatcher {
public:
    Inter_MonitorNoticeGuildCreate();
    virtual ~Inter_MonitorNoticeGuildCreate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorGuildMemberNamingReply : public ISigDispatcher {
public:
    Inter_MonitorGuildMemberNamingReply();
    virtual ~Inter_MonitorGuildMemberNamingReply();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorSetGuildKey) == 0x04u,
              "Inter_MonitorSetGuildKey must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildCreate) == 0x04u,
              "Inter_MonitorNoticeGuildCreate must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorGuildMemberNamingReply) == 0x04u,
              "Inter_MonitorGuildMemberNamingReply must retain the base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_GUILD_MONITOR_STATE_DISPATCHERS_A_H_
