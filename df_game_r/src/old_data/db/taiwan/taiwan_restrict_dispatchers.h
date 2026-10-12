#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RESTRICT_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RESTRICT_DISPATCHERS_H_

#include <stdint.h>

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"


class Stream;


// Taiwan internal code 1.  The old object has no storage beyond the
// TaiwanInterfaceDBDispatcher [vptr, code] base.
class LoadServiceRestrictPolicy : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815DFF8
    // IDA_NAME: _ZN25LoadServiceRestrictPolicyC2Ev
    // DEMANGLED: LoadServiceRestrictPolicy::LoadServiceRestrictPolicy(void)
    LoadServiceRestrictPolicy();

    // IDA_EA: 0x815E01C (D2), 0x815E04C (D0)
    // IDA_NAME: _ZN25LoadServiceRestrictPolicyD2Ev / D0Ev
    virtual ~LoadServiceRestrictPolicy();

    // IDA_EA: 0x815A8F2
    // IDA_NAME: _ZN25LoadServiceRestrictPolicy8dispatchEiiP6Stream
    // DEMANGLED: LoadServiceRestrictPolicy::dispatch(int,int,Stream *)
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x815AC40
    // IDA_NAME: _ZN25LoadServiceRestrictPolicy16sendRestrictDataERN6Taiwan21SigLoadRestrictPolicyE
    // DEMANGLED: LoadServiceRestrictPolicy::sendRestrictData(Taiwan::SigLoadRestrictPolicy &)
    void sendRestrictData(Taiwan::SigLoadRestrictPolicy& policy);
};


// Taiwan internal code 2.  The save receiver is also base-only on i386.
class SaveServiceRestrictPolicy : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E06A
    // IDA_NAME: _ZN25SaveServiceRestrictPolicyC2Ev
    // DEMANGLED: SaveServiceRestrictPolicy::SaveServiceRestrictPolicy(void)
    SaveServiceRestrictPolicy();

    // IDA_EA: 0x815E08E (D2), 0x815E0BE (D0)
    // IDA_NAME: _ZN25SaveServiceRestrictPolicyD2Ev / D0Ev
    virtual ~SaveServiceRestrictPolicy();

    // IDA_EA: 0x815AD3C
    // IDA_NAME: _ZN25SaveServiceRestrictPolicy8dispatchEiiP6Stream
    // DEMANGLED: SaveServiceRestrictPolicy::dispatch(int,int,Stream *)
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};


// Taiwan internal code 4.  It reads geo_allow_country in the original 44-row
// chunks and returns each chunk through the internal dispatcher queue.
class LoadGeolocation : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E0DC
    // IDA_NAME: _ZN15LoadGeolocationC2Ev
    // DEMANGLED: LoadGeolocation::LoadGeolocation(void)
    LoadGeolocation();

    // IDA_EA: 0x815E100 (D2), 0x815E130 (D0)
    // IDA_NAME: _ZN15LoadGeolocationD2Ev / D0Ev
    virtual ~LoadGeolocation();

    // IDA_EA: 0x815B010
    // IDA_NAME: _ZN15LoadGeolocation8dispatchEiiP6Stream
    // DEMANGLED: LoadGeolocation::dispatch(int,int,Stream *)
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x815B266
    // IDA_NAME: _ZN15LoadGeolocation15sendCountryDataERN6Taiwan18SigLoadGeolocationE
    // DEMANGLED: LoadGeolocation::sendCountryData(Taiwan::SigLoadGeolocation &)
    void sendCountryData(Taiwan::SigLoadGeolocation& countries);
};


// Taiwan internal code 8.  makeRequest is a static three-argument producer
// (account_id, source user index, timestamp), not a stateful DB object method.
class DBUpdateExitTime : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E232
    // IDA_NAME: _ZN16DBUpdateExitTimeC2Ev
    // DEMANGLED: DBUpdateExitTime::DBUpdateExitTime(void)
    DBUpdateExitTime();

    // IDA_EA: 0x815E256 (D2), 0x815E286 (D0)
    // IDA_NAME: _ZN16DBUpdateExitTimeD2Ev / D0Ev
    virtual ~DBUpdateExitTime();

    // IDA_EA: 0x815B89A
    // IDA_NAME: _ZN16DBUpdateExitTime11makeRequestEjij
    // DEMANGLED: DBUpdateExitTime::makeRequest(unsigned int,int,unsigned int)
    static void makeRequest(uint32_t account_id,
                            int source_user_idx,
                            uint32_t timestamp);

    // IDA_EA: 0x815B97C
    // IDA_NAME: _ZN16DBUpdateExitTime8dispatchEiiP6Stream
    // DEMANGLED: DBUpdateExitTime::dispatch(int,int,Stream *)
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};


#if defined(__i386__)
static_assert(sizeof(LoadServiceRestrictPolicy) == 0x08u,
              "LoadServiceRestrictPolicy must retain the Taiwan DB ABI");
static_assert(sizeof(SaveServiceRestrictPolicy) == 0x08u,
              "SaveServiceRestrictPolicy must retain the Taiwan DB ABI");
static_assert(sizeof(LoadGeolocation) == 0x08u,
              "LoadGeolocation must retain the Taiwan DB ABI");
static_assert(sizeof(DBUpdateExitTime) == 0x08u,
              "DBUpdateExitTime must retain the Taiwan DB ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RESTRICT_DISPATCHERS_H_
