#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_LIVE_EVENT_DATA_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_LIVE_EVENT_DATA_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/Inter_LoadPvPLiveEventData.
class Inter_LoadPvPLiveEventData : public ISigDispatcher {
public:
    Inter_LoadPvPLiveEventData();
    virtual ~Inter_LoadPvPLiveEventData();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadPvPLiveEventData) == 0x04u,
              "Inter_LoadPvPLiveEventData must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_LIVE_EVENT_DATA_RUNTIME_H_
