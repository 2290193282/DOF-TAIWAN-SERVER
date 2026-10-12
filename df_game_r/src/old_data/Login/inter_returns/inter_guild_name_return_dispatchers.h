#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NAME_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NAME_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_ChangeGuildName : public ISigDispatcher {
public:
    Inter_ChangeGuildName();
    virtual ~Inter_ChangeGuildName();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeChangeGuildNameToMembers : public ISigDispatcher {
public:
    Inter_NoticeChangeGuildNameToMembers();
    virtual ~Inter_NoticeChangeGuildNameToMembers();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeGuildNameOnGuildCreate : public ISigDispatcher {
public:
    Inter_NoticeGuildNameOnGuildCreate();
    virtual ~Inter_NoticeGuildNameOnGuildCreate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ChangeGuildName) == 0x04u &&
                  sizeof(Inter_NoticeChangeGuildNameToMembers) == 0x04u &&
                  sizeof(Inter_NoticeGuildNameOnGuildCreate) == 0x04u,
              "Guild-name Inter leaves must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NAME_RETURN_DISPATCHERS_H_
