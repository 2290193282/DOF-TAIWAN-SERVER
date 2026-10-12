#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_HERO_MISSION_EVENT_DATA_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_HERO_MISSION_EVENT_DATA_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class Stream;

// Old Taiwan DB handler, route code 18.  It reads a character's persisted
// hero-mission blob and returns it through TaiwanInternalDispatcher.
class DBLoadHeroMissionEventData : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E5C2
    // IDA_NAME: _ZN26DBLoadHeroMissionEventDataC2Ev
    DBLoadHeroMissionEventData();

    // IDA_EA: 0x815E5E6 (D2), 0x815E616 (D0)
    // IDA_NAME: _ZN26DBLoadHeroMissionEventDataD2Ev / D0Ev
    virtual ~DBLoadHeroMissionEventData();

    // IDA_EA: 0x815D7A8
    // IDA_NAME: _ZN26DBLoadHeroMissionEventData8dispatchEiiP6Stream
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

// Old Taiwan DB handler, route code 17.  It stores the typed 0x104-byte
// hero-mission payload received from the HeroMissionEvent producer.
class DBSaveHeroMissionEventData : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E634
    // IDA_NAME: _ZN26DBSaveHeroMissionEventDataC2Ev
    DBSaveHeroMissionEventData();

    // IDA_EA: 0x815E658 (D2), 0x815E688 (D0)
    // IDA_NAME: _ZN26DBSaveHeroMissionEventDataD2Ev / D0Ev
    virtual ~DBSaveHeroMissionEventData();

    // IDA_EA: 0x815DB98
    // IDA_NAME: _ZN26DBSaveHeroMissionEventData8dispatchEiiP6Stream
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DBLoadHeroMissionEventData) == 0x08u,
              "DBLoadHeroMissionEventData must retain the Taiwan DB ABI");
static_assert(sizeof(DBSaveHeroMissionEventData) == 0x08u,
              "DBSaveHeroMissionEventData must retain the Taiwan DB ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_HERO_MISSION_EVENT_DATA_H_
