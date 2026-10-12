#ifndef OLDSRV_OLD_GAME_EVENT_INTER_SERVER_EVENT_START_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_SERVER_EVENT_START_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;

// IDA_EA: Inter_MonitorServerEventStart::dispatch_sig @ 0x84CF3CA.
// The monitor event-start frame is consumed as one packed 18-byte record.
// It is not an opaque tail: the two words feed CServerEvent and the timer
// queue as the experience-rate delta and event interval respectively.
#pragma pack(push, 1)
struct Packet_Monitor_ServerEventStart {
    PacketHeader header;              // +0x00
    uint32_t event_id;                // +0x0a
    uint16_t exp_rate_delta;          // +0x0e
    uint16_t event_interval_seconds;  // +0x10
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_ServerEventStart) == 0x12u,
              "server-event start packet must retain its 18-byte ABI");
static_assert(offsetof(Packet_Monitor_ServerEventStart, event_id) == 0x0au &&
                  offsetof(Packet_Monitor_ServerEventStart, exp_rate_delta) ==
                      0x0eu &&
                  offsetof(Packet_Monitor_ServerEventStart,
                           event_interval_seconds) == 0x10u,
              "server-event start packet field offsets changed");

class Inter_MonitorServerEventStart : public ISigDispatcher {
public:
    // IDA_EA: 0x84CF2DE
    // IDA_NAME: _ZN29Inter_MonitorServerEventStart19SendEventNotiPacketEbssP5CUser
    // DEMANGLED: Inter_MonitorServerEventStart::SendEventNotiPacket(bool,short,short,CUser *)
    static void SendEventNotiPacket(bool event_started,
                                    int16_t event_id,
                                    int16_t event_interval,
                                    CUser* user);

    // IDA_EA: 0x84CF3CA
    // IDA_NAME: _ZN29Inter_MonitorServerEventStart12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorServerEventStart::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED4EC / 0x84F9E8E / 0x84F9EBE
    Inter_MonitorServerEventStart();
    virtual ~Inter_MonitorServerEventStart();
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorServerEventStart) == 0x04u,
              "Inter_MonitorServerEventStart must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_SERVER_EVENT_START_H_
