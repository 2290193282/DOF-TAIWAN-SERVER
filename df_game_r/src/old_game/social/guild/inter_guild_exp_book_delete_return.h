#ifndef OLDSRV_OLD_GAME_GROUP_INTER_GUILD_EXP_BOOK_DELETE_RETURN_H_
#define OLDSRV_OLD_GAME_GROUP_INTER_GUILD_EXP_BOOK_DELETE_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_GuildExpBookDelete : public ISigDispatcher {
public:
    Inter_GuildExpBookDelete();
    virtual ~Inter_GuildExpBookDelete();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_GuildExpBookDelete) == 0x04u,
              "Inter_GuildExpBookDelete must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_INTER_GUILD_EXP_BOOK_DELETE_RETURN_H_
