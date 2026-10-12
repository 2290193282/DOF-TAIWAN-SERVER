#ifndef OLDSRV_LOGIN_INTER_MONITOR_USER_GETOUT_H_
#define OLDSRV_LOGIN_INTER_MONITOR_USER_GETOUT_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;

// IDA_EA: Inter_MonitorUserGetout::dispatch_sig @ 0x84C8B0C.  The
// monitor-side frame carries only the common 10-byte PacketHeader followed
// by the account number read at +0x0A.
#pragma pack(push, 1)
struct Packet_Monitor_UserGetout {
    PacketHeader header;       // +0x00
    uint32_t account_id;       // +0x0A
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_UserGetout) == 0x0Eu,
              "monitor user-getout packet must retain its 14-byte ABI");
static_assert(offsetof(Packet_Monitor_UserGetout, account_id) == 0x0Au,
              "monitor user-getout account id offset changed");

class Inter_MonitorUserGetout : public ISigDispatcher {
public:
    // IDA_EA: 0x84C8B0C
    // IDA_NAME: _ZN23Inter_MonitorUserGetout12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EA282 / 0x84EA29E / 0x84EA2CE
    Inter_MonitorUserGetout();
    virtual ~Inter_MonitorUserGetout();
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorUserGetout) == 0x04u,
              "Inter_MonitorUserGetout must remain vptr-only on i386");
#endif

#endif  // OLDSRV_LOGIN_INTER_MONITOR_USER_GETOUT_H_
