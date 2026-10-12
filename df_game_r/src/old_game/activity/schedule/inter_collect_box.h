#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_INTER_COLLECT_BOX_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_INTER_COLLECT_BOX_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class Inter_CollectBox : public ISigDispatcher {
public:
    Inter_CollectBox();
    virtual ~Inter_CollectBox();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CollectBox) == 0x04u,
              "CollectBox Inter dispatcher must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_INTER_COLLECT_BOX_0828_H_
