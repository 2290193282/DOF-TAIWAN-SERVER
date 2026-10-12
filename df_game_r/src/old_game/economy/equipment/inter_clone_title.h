#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_INTER_CLONE_TITLE_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_INTER_CLONE_TITLE_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class Inter_CloneTitle : public ISigDispatcher {
public:
    Inter_CloneTitle();
    virtual ~Inter_CloneTitle();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CloneTitle) == 0x04u,
              "clone-title inter dispatcher must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_INTER_CLONE_TITLE_0828_H_
