#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_TOWER_OF_DESPAIR_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_TOWER_OF_DESPAIR_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Slot 630.  The Tower random APC selection completion is forwarded to the
// monitor endpoint for this game-server group.
class Inter_TOD_ReturnToMonitor_APCSettingComplete : public ISigDispatcher {
public:
    Inter_TOD_ReturnToMonitor_APCSettingComplete();
    virtual ~Inter_TOD_ReturnToMonitor_APCSettingComplete();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// Slot 631.  A monitor refresh contains ten raw _APCInfo records and replaces
// the Tower-of-Despair APC selection table atomically from the receiver's
// perspective.
class Inter_TOD_Reload_APC : public ISigDispatcher {
public:
    Inter_TOD_Reload_APC();
    virtual ~Inter_TOD_Reload_APC();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_TOD_ReturnToMonitor_APCSettingComplete) == 0x04u &&
                  sizeof(Inter_TOD_Reload_APC) == 0x04u,
              "Tower Inter leaves must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_TOWER_OF_DESPAIR_DISPATCHERS_H_
