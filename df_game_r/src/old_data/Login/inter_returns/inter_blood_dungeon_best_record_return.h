#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BLOOD_DUNGEON_BEST_RECORD_RETURN_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BLOOD_DUNGEON_BEST_RECORD_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 09_dungeon_pvp_party/Inter_LoadBloodDungeonBestRecord.
class Inter_LoadBloodDungeonBestRecord : public ISigDispatcher {
public:
    Inter_LoadBloodDungeonBestRecord();
    virtual ~Inter_LoadBloodDungeonBestRecord();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadBloodDungeonBestRecord) == 0x04u,
              "Inter_LoadBloodDungeonBestRecord must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BLOOD_DUNGEON_BEST_RECORD_RETURN_H_
