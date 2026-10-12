#ifndef OLDSRV_GLOBAL_GUILD_JOIN_INTER_RECEIVERS_H_
#define OLDSRV_GLOBAL_GUILD_JOIN_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// IDA units: 99_misc_global/Inter_CheckJoinGuild,
// 99_misc_global/Inter_JoinGuildInfo, and
// 99_misc_global/Inter_GuildJoinList. These real reply receivers remain
// unregistered until the original complete InterDispatcher table is restored.
class Inter_CheckJoinGuild : public ISigDispatcher {
public:
    Inter_CheckJoinGuild();
    virtual ~Inter_CheckJoinGuild();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_JoinGuildInfo : public ISigDispatcher {
public:
    Inter_JoinGuildInfo();
    virtual ~Inter_JoinGuildInfo();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GuildJoinList : public ISigDispatcher {
public:
    Inter_GuildJoinList();
    virtual ~Inter_GuildJoinList();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 08_quest_skill_growth/Inter_RequestJoinGuild.  The request's DB
// reply is a distinct 0x48-byte record, so it is not interchangeable with
// the existing check/join-list reply handlers above.
class Inter_RequestJoinGuild : public ISigDispatcher {
public:
    Inter_RequestJoinGuild();
    virtual ~Inter_RequestJoinGuild();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 99_misc_global/Inter_AddGuildFundResult. The guild service sends
// a packed 0x17-byte result: byte +0x0a is the result code, dword +0x0b is
// charac_no, dword +0x0f is the routing UID and dword +0x13 is the amount to
// return on failure. Like the other guild receivers, root InterDispatcher
// registration remains a separate closure.
class Inter_AddGuildFundResult : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE9B4
    // IDA_NAME: _ZN24Inter_AddGuildFundResultC2Ev
    // DEMANGLED: Inter_AddGuildFundResult::Inter_AddGuildFundResult(void)
    Inter_AddGuildFundResult();

    // IDA_EA: 0x84F6804 / 0x84F6834
    // IDA_NAME: _ZN24Inter_AddGuildFundResultD2Ev /
    //           _ZN24Inter_AddGuildFundResultD0Ev
    // DEMANGLED: Inter_AddGuildFundResult::~Inter_AddGuildFundResult()
    virtual ~Inter_AddGuildFundResult();

    // IDA_EA: 0x84E34BA
    // IDA_NAME: _ZN24Inter_AddGuildFundResult12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_AddGuildFundResult::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#endif  // OLDSRV_GLOBAL_GUILD_JOIN_INTER_RECEIVERS_H_
