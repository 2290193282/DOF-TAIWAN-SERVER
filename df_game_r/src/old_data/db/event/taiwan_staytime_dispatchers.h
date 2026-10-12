#ifndef OLDSRV_OLD_DATA_DB_EVENT_TAIWAN_STAYTIME_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_EVENT_TAIWAN_STAYTIME_DISPATCHERS_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class Stream;

// Taiwan internal code 5.  This object has no fields beyond the old
// TaiwanInterfaceDBDispatcher [vptr, code] base.
class CheckStayTimeEvent : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E14E
    // IDA_NAME: _ZN18CheckStayTimeEventC2Ev
    CheckStayTimeEvent();

    // IDA_EA: 0x815E172 (D2), 0x815E1A2 (D0)
    // IDA_NAME: _ZN18CheckStayTimeEventD2Ev / D0Ev
    virtual ~CheckStayTimeEvent();

    // IDA_EA: 0x815B362
    // IDA_NAME: _ZN18CheckStayTimeEvent8dispatchEiiP6Stream
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x815B6AC
    // IDA_NAME: _ZN18CheckStayTimeEvent16sendStayTimeDataEi
    void sendStayTimeData(int user_idx);
};

// Taiwan internal code 7.  The DB trigger reserves a four-byte payload but
// the original receiver does not read it; it resets every reward flag.
class ResetStayTimeEvent : public TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815E1C0
    // IDA_NAME: _ZN18ResetStayTimeEventC2Ev
    ResetStayTimeEvent();

    // IDA_EA: 0x815E1E4 (D2), 0x815E214 (D0)
    // IDA_NAME: _ZN18ResetStayTimeEventD2Ev / D0Ev
    virtual ~ResetStayTimeEvent();

    // IDA_EA: 0x815B76A
    // IDA_NAME: _ZN18ResetStayTimeEvent8dispatchEiiP6Stream
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(CheckStayTimeEvent) == 0x08u,
              "CheckStayTimeEvent must retain the Taiwan DB ABI");
static_assert(sizeof(ResetStayTimeEvent) == 0x08u,
              "ResetStayTimeEvent must retain the Taiwan DB ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_EVENT_TAIWAN_STAYTIME_DISPATCHERS_H_
