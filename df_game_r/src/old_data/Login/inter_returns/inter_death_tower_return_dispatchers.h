#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_DEATH_TOWER_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_DEATH_TOWER_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Original InterDispatcher return consumers for SIG_DEATH_TOWER_*.
// All four remain base-only ISigDispatcher objects in the i386 server ABI.
class Inter_LoadDeathTowerBestRecord : public ISigDispatcher {
public:
    Inter_LoadDeathTowerBestRecord();
    virtual ~Inter_LoadDeathTowerBestRecord();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_DeathTowerLoadTopRanker : public ISigDispatcher {
public:
    Inter_DeathTowerLoadTopRanker();
    virtual ~Inter_DeathTowerLoadTopRanker();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorDeathTowerRanking : public ISigDispatcher {
public:
    Inter_MonitorDeathTowerRanking();
    virtual ~Inter_MonitorDeathTowerRanking();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorTowerReloadTopRanker : public ISigDispatcher {
public:
    Inter_MonitorTowerReloadTopRanker();
    virtual ~Inter_MonitorTowerReloadTopRanker();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadDeathTowerBestRecord) == 0x04u,
              "Inter_LoadDeathTowerBestRecord must retain the original base-only ABI");
static_assert(sizeof(Inter_DeathTowerLoadTopRanker) == 0x04u,
              "Inter_DeathTowerLoadTopRanker must retain the original base-only ABI");
static_assert(sizeof(Inter_MonitorDeathTowerRanking) == 0x04u,
              "Inter_MonitorDeathTowerRanking must retain the original base-only ABI");
static_assert(sizeof(Inter_MonitorTowerReloadTopRanker) == 0x04u,
              "Inter_MonitorTowerReloadTopRanker must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_DEATH_TOWER_RETURN_DISPATCHERS_H_
