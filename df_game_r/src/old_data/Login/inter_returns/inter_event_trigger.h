#ifndef OLDSRV_LOGIN_INTER_EVENT_TRIGGER_H_
#define OLDSRV_LOGIN_INTER_EVENT_TRIGGER_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/activity/reward/event_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;

// The monitor-to-game event-control replies retain the old 10-byte internal
// packet header.  The receiver EAs below read their bodies at +0x0A, which is
// sizeof(PacketHeader), rather than at an ad-hoc raw offset.
#pragma pack(push, 1)
struct Packet_Monitor_TriggerEventStart : public PacketHeader {
    int32_t event_id;     // +0x0A
    Word_Param parameter; // +0x0E
};

struct Packet_Monitor_TriggerEventEnd : public PacketHeader {
    int32_t event_id;  // +0x0A
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_TriggerEventStart) == 0x12u,
              "monitor event-start packet must retain its old 18-byte ABI");
static_assert(offsetof(Packet_Monitor_TriggerEventStart, event_id) == 0x0Au &&
                  offsetof(Packet_Monitor_TriggerEventStart, parameter) == 0x0Eu,
              "monitor event-start packet offsets changed");
static_assert(sizeof(Packet_Monitor_TriggerEventEnd) == 0x0Eu,
              "monitor event-end packet must retain its old 14-byte ABI");
static_assert(offsetof(Packet_Monitor_TriggerEventEnd, event_id) == 0x0Au,
              "monitor event-end packet offset changed");

class Inter_TriggerEventStart : public ISigDispatcher {
public:
    // IDA_EA: 0x84CE5F4
    // IDA_NAME: _ZN23Inter_TriggerEventStart12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EB1EE / 0x84EB20A / 0x84EB23A
    Inter_TriggerEventStart();
    virtual ~Inter_TriggerEventStart();
};

class Inter_TriggerEventEnd : public ISigDispatcher {
public:
    // IDA_EA: 0x84CE646
    // IDA_NAME: _ZN21Inter_TriggerEventEnd12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EB258 / 0x84EB274 / 0x84EB2A4
    Inter_TriggerEventEnd();
    virtual ~Inter_TriggerEventEnd();
};

#if defined(__i386__)
static_assert(sizeof(Inter_TriggerEventStart) == 0x04u,
              "Inter_TriggerEventStart must retain its base-only i386 ABI");
static_assert(sizeof(Inter_TriggerEventEnd) == 0x04u,
              "Inter_TriggerEventEnd must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_EVENT_TRIGGER_H_
