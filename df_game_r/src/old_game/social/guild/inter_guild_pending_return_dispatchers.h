#ifndef OLDSRV_OLD_GAME_GROUP_INTER_GUILD_PENDING_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_GROUP_INTER_GUILD_PENDING_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// These are one-vptr InterDispatcher objects in the original i386 runtime.
class Inter_CallGuildInviteToInvited : public ISigDispatcher {
public:
    Inter_CallGuildInviteToInvited();
    virtual ~Inter_CallGuildInviteToInvited();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReplyGuildSecede : public ISigDispatcher {
public:
    Inter_ReplyGuildSecede();
    virtual ~Inter_ReplyGuildSecede();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReplyGuildCreate : public ISigDispatcher {
public:
    Inter_ReplyGuildCreate();
    virtual ~Inter_ReplyGuildCreate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CallGuildInviteToInvited) == 0x04u,
              "Inter_CallGuildInviteToInvited must retain base-only ABI");
static_assert(sizeof(Inter_ReplyGuildSecede) == 0x04u,
              "Inter_ReplyGuildSecede must retain base-only ABI");
static_assert(sizeof(Inter_ReplyGuildCreate) == 0x04u,
              "Inter_ReplyGuildCreate must retain base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_INTER_GUILD_PENDING_RETURN_DISPATCHERS_H_
