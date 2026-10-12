#ifndef OLDSRV_GLOBAL_GUILD_MONITOR_QUERY_DISPATCHERS_A_H_
#define OLDSRV_GLOBAL_GUILD_MONITOR_QUERY_DISPATCHERS_A_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// The four monitor replies below are base-only ISigDispatcher instances in
// the i386 df_game_r.  They are kept together because their incoming wire
// records share the same Monitor recipient prefix and guild-query ABI.
class Inter_MonitorCallGuildMembers : public ISigDispatcher {
public:
    Inter_MonitorCallGuildMembers();
    virtual ~Inter_MonitorCallGuildMembers();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildNames : public ISigDispatcher {
public:
    Inter_MonitorNoticeGuildNames();
    virtual ~Inter_MonitorNoticeGuildNames();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorCallGuildInfo : public ISigDispatcher {
public:
    Inter_MonitorCallGuildInfo();
    virtual ~Inter_MonitorCallGuildInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildInfo : public ISigDispatcher {
public:
    Inter_MonitorNoticeGuildInfo();
    virtual ~Inter_MonitorNoticeGuildInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorCallGuildMembers) == 0x04u,
              "Inter_MonitorCallGuildMembers must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildNames) == 0x04u,
              "Inter_MonitorNoticeGuildNames must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorCallGuildInfo) == 0x04u,
              "Inter_MonitorCallGuildInfo must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildInfo) == 0x04u,
              "Inter_MonitorNoticeGuildInfo must retain the base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_GUILD_MONITOR_QUERY_DISPATCHERS_A_H_
