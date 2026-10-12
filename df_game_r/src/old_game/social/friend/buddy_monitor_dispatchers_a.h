#ifndef OLDSRV_GLOBAL_BUDDY_MONITOR_DISPATCHERS_A_H_
#define OLDSRV_GLOBAL_BUDDY_MONITOR_DISPATCHERS_A_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 99_misc_global/Inter_MonitorBuddyInOut.  The incoming monitor
// signal owns all state; this object is only the original base-only receiver.
class Inter_MonitorBuddyInOut : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA42A
    // IDA_NAME: _ZN23Inter_MonitorBuddyInOutC2Ev
    Inter_MonitorBuddyInOut();

    // IDA_EA: 0x84EA446 / 0x84EA476
    // IDA_NAME: _ZN23Inter_MonitorBuddyInOutD2Ev /
    //           _ZN23Inter_MonitorBuddyInOutD0Ev
    virtual ~Inter_MonitorBuddyInOut();

    // IDA_EA: 0x84C8EC0
    // IDA_NAME: _ZN23Inter_MonitorBuddyInOut12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorBuddyInOut::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 99_misc_global/Inter_MonitorReplyBuddyList.  The payload's
// account id and trailing 42-byte BuddyList records are forwarded through the
// canonical GameWorld sender; this class intentionally has no owned state.
class Inter_MonitorReplyBuddyList : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA494
    // IDA_NAME: _ZN27Inter_MonitorReplyBuddyListC2Ev
    Inter_MonitorReplyBuddyList();

    // IDA_EA: 0x84EA4B0 / 0x84EA4E0
    // IDA_NAME: _ZN27Inter_MonitorReplyBuddyListD2Ev /
    //           _ZN27Inter_MonitorReplyBuddyListD0Ev
    virtual ~Inter_MonitorReplyBuddyList();

    // IDA_EA: 0x84C9148
    // IDA_NAME: _ZN27Inter_MonitorReplyBuddyList12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorReplyBuddyList::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorBuddyInOut) == 0x04u,
              "Inter_MonitorBuddyInOut must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorReplyBuddyList) == 0x04u,
              "Inter_MonitorReplyBuddyList must retain the base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_BUDDY_MONITOR_DISPATCHERS_A_H_
