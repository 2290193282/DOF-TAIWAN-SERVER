#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_REPORT_MANNERLESS_RETURN_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_REPORT_MANNERLESS_RETURN_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Database/Monitor return for CMD140's complete
// Packet_Mannerless_User_Accusation record.
class Inter_ReportMannerless : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED8C0
    // IDA_NAME: _ZN22Inter_ReportMannerlessC2Ev
    Inter_ReportMannerless();

    // IDA_EA: 0x84F956A
    // IDA_NAME: _ZN22Inter_ReportMannerlessD2Ev
    // IDA_EA: 0x84F959A
    // IDA_NAME: _ZN22Inter_ReportMannerlessD0Ev
    virtual ~Inter_ReportMannerless();

    // IDA_EA: 0x84D24A2
    // IDA_NAME: _ZN22Inter_ReportMannerless12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ReportMannerless) == 0x04u,
              "Inter_ReportMannerless must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_REPORT_MANNERLESS_RETURN_DISPATCHER_H_
