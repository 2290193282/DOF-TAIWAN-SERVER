#ifndef OLDSRV_OLD_GAME_PLAYER_CORE_INTER_CHARAC_LINK_H_
#define OLDSRV_OLD_GAME_PLAYER_CORE_INTER_CHARAC_LINK_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class Inter_CharacLink : public ISigDispatcher {
public:
    Inter_CharacLink();
    virtual ~Inter_CharacLink();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CharacLink) == 0x04u,
              "character-link Inter dispatcher must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_PLAYER_CORE_INTER_CHARAC_LINK_0828_H_
