#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SOCIAL_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SOCIAL_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_MonitorAddBuddy : public ISigDispatcher {
public:
    Inter_MonitorAddBuddy();
    virtual ~Inter_MonitorAddBuddy();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorDelBuddy : public ISigDispatcher {
public:
    Inter_MonitorDelBuddy();
    virtual ~Inter_MonitorDelBuddy();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ChangeLetterStat : public ISigDispatcher {
public:
    Inter_ChangeLetterStat();
    virtual ~Inter_ChangeLetterStat();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReqQueryCharacInfo : public ISigDispatcher {
public:
    Inter_ReqQueryCharacInfo();
    virtual ~Inter_ReqQueryCharacInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeChangeGuildMemberGrade : public ISigDispatcher {
public:
    Inter_MonitorNoticeChangeGuildMemberGrade();
    virtual ~Inter_MonitorNoticeChangeGuildMemberGrade();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GuildNoticeGuildMessage : public ISigDispatcher {
public:
    Inter_GuildNoticeGuildMessage();
    virtual ~Inter_GuildNoticeGuildMessage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorAddBuddy) == 0x04u,
              "Inter_MonitorAddBuddy must retain the original base-only ABI");
static_assert(sizeof(Inter_MonitorDelBuddy) == 0x04u,
              "Inter_MonitorDelBuddy must retain the original base-only ABI");
static_assert(sizeof(Inter_ChangeLetterStat) == 0x04u,
              "Inter_ChangeLetterStat must retain the original base-only ABI");
static_assert(sizeof(Inter_ReqQueryCharacInfo) == 0x04u,
              "Inter_ReqQueryCharacInfo must retain the original base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeChangeGuildMemberGrade) == 0x04u,
              "Inter_MonitorNoticeChangeGuildMemberGrade must retain the original base-only ABI");
static_assert(sizeof(Inter_GuildNoticeGuildMessage) == 0x04u,
              "Inter_GuildNoticeGuildMessage must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SOCIAL_RETURN_DISPATCHERS_H_
