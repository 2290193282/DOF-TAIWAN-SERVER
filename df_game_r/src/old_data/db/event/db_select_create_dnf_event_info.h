#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_CREATE_DNF_EVENT_INFO_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_CREATE_DNF_EVENT_INFO_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class Stream;

// IDA unit: 11_event_runtime/DBSelectCreateDnfEventInfo.
// IDA_EA: makeRequest/dispatch @0x815BA94/@0x815BB8C.
struct DBSelectCreateDnfEventInfo : public TaiwanInterfaceDBDispatcher {
    // IDA_EA: 0x815E2A4
    // IDA_NAME: _ZN26DBSelectCreateDnfEventInfoC2Ev
    DBSelectCreateDnfEventInfo();

    // IDA_EA: 0x815E2C8 (D2), 0x815E2F8 (D0)
    // IDA_NAME: _ZN26DBSelectCreateDnfEventInfoD2Ev / D0Ev
    virtual ~DBSelectCreateDnfEventInfo();

    static void makeRequest();
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DBSelectCreateDnfEventInfo) == 0x08u,
              "DBSelectCreateDnfEventInfo must not extend the Taiwan DB ABI");
#endif

#endif
