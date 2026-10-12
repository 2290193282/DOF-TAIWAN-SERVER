#ifndef OLDSRV_GLOBAL_GUILD_MONITOR_NOTICE_DISPATCHERS_A_H_
#define OLDSRV_GLOBAL_GUILD_MONITOR_NOTICE_DISPATCHERS_A_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA units: 99_misc_global/Inter_MonitorNoticeGuild{Enter,Secede,
// MarkChange,Dismiss,ChatMsg}.  These are the original monitor/guild-server
// reply receivers.  They intentionally retain the base-only ISigDispatcher
// object ABI: all signal payload state belongs to the incoming buffer.
class Inter_MonitorNoticeGuildEnter : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA4FE
    // IDA_NAME: _ZN29Inter_MonitorNoticeGuildEnterC2Ev
    Inter_MonitorNoticeGuildEnter();

    // IDA_EA: 0x84EA51A / 0x84EA54A
    // IDA_NAME: _ZN29Inter_MonitorNoticeGuildEnterD2Ev /
    //           _ZN29Inter_MonitorNoticeGuildEnterD0Ev
    virtual ~Inter_MonitorNoticeGuildEnter();

    // IDA_EA: 0x84C9194
    // IDA_NAME: _ZN29Inter_MonitorNoticeGuildEnter12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildSecede : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA5D2
    // IDA_NAME: _ZN30Inter_MonitorNoticeGuildSecedeC2Ev
    Inter_MonitorNoticeGuildSecede();

    // IDA_EA: 0x84EA5EE / 0x84EA61E
    // IDA_NAME: _ZN30Inter_MonitorNoticeGuildSecedeD2Ev /
    //           _ZN30Inter_MonitorNoticeGuildSecedeD0Ev
    virtual ~Inter_MonitorNoticeGuildSecede();

    // IDA_EA: 0x84C957E
    // IDA_NAME: _ZN30Inter_MonitorNoticeGuildSecede12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildMarkChange : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA63C
    // IDA_NAME: _ZN34Inter_MonitorNoticeGuildMarkChangeC2Ev
    Inter_MonitorNoticeGuildMarkChange();

    // IDA_EA: 0x84EA658 / 0x84EA688
    // IDA_NAME: _ZN34Inter_MonitorNoticeGuildMarkChangeD2Ev /
    //           _ZN34Inter_MonitorNoticeGuildMarkChangeD0Ev
    virtual ~Inter_MonitorNoticeGuildMarkChange();

    // IDA_EA: 0x84CA1E6
    // IDA_NAME: _ZN34Inter_MonitorNoticeGuildMarkChange12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildDismiss : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA6A6
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildDismissC2Ev
    Inter_MonitorNoticeGuildDismiss();

    // IDA_EA: 0x84EA6C2 / 0x84EA6F2
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildDismissD2Ev /
    //           _ZN31Inter_MonitorNoticeGuildDismissD0Ev
    virtual ~Inter_MonitorNoticeGuildDismiss();

    // IDA_EA: 0x84CA00C
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildDismiss12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildChatMsg : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA710
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildChatMsgC2Ev
    Inter_MonitorNoticeGuildChatMsg();

    // IDA_EA: 0x84EA72C / 0x84EA75C
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildChatMsgD2Ev /
    //           _ZN31Inter_MonitorNoticeGuildChatMsgD0Ev
    virtual ~Inter_MonitorNoticeGuildChatMsg();

    // IDA_EA: 0x84C9E30
    // IDA_NAME: _ZN31Inter_MonitorNoticeGuildChatMsg12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorNoticeGuildEnter) == 0x04u,
              "Inter_MonitorNoticeGuildEnter must retain base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildSecede) == 0x04u,
              "Inter_MonitorNoticeGuildSecede must retain base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildMarkChange) == 0x04u,
              "Inter_MonitorNoticeGuildMarkChange must retain base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildDismiss) == 0x04u,
              "Inter_MonitorNoticeGuildDismiss must retain base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildChatMsg) == 0x04u,
              "Inter_MonitorNoticeGuildChatMsg must retain base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_GUILD_MONITOR_NOTICE_DISPATCHERS_A_H_
