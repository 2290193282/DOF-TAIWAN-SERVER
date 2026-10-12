#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_CALL_USERINFO_RETURN_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_CALL_USERINFO_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 06_userinfo_stat_inventory/Inter_MonitorCallUserInfo.
class Inter_MonitorCallUserInfo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA3C0 / 0x84EA3DC / 0x84EA40C
    Inter_MonitorCallUserInfo();
    virtual ~Inter_MonitorCallUserInfo();

    // IDA_EA: 0x84C89B2
    // IDA_NAME: _ZN25Inter_MonitorCallUserInfo12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorCallUserInfo) == 0x04u,
              "Inter_MonitorCallUserInfo must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_CALL_USERINFO_RETURN_H_
