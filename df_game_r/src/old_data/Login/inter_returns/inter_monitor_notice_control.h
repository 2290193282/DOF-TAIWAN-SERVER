#ifndef OLDSRV_LOGIN_INTER_MONITOR_NOTICE_CONTROL_H_
#define OLDSRV_LOGIN_INTER_MONITOR_NOTICE_CONTROL_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;

// Monitor TCP messages retain the 10-byte internal PacketHeader.  Both bodies
// are variable-length protocol strings, represented as named flexible tails
// whose length is carried by the preceding field; no opaque fixed tail is
// introduced.
#pragma pack(push, 1)
struct Packet_Monitor_NoticeMessage {
    PacketHeader header;  // +0x00
    uint8_t message_length;  // +0x0A
    char message[0];      // +0x0B, message_length bytes
};

struct Packet_Monitor_ForbidChat {
    PacketHeader header;        // +0x00
    int32_t forbid_seconds;     // +0x0A
    uint32_t charac_name_length; // +0x0E
    char charac_name[0];        // +0x12, charac_name_length bytes
};
#pragma pack(pop)

static_assert(offsetof(Packet_Monitor_NoticeMessage, message_length) == 0x0Au &&
                  offsetof(Packet_Monitor_NoticeMessage, message) == 0x0Bu,
              "monitor notice packet body offsets changed");
static_assert(offsetof(Packet_Monitor_ForbidChat, forbid_seconds) == 0x0Au &&
                  offsetof(Packet_Monitor_ForbidChat, charac_name_length) == 0x0Eu &&
                  offsetof(Packet_Monitor_ForbidChat, charac_name) == 0x12u,
              "monitor forbid-chat packet body offsets changed");

class Inter_MonitorNoticeMessage : public ISigDispatcher {
public:
    // IDA_EA: 0x84D035E
    // IDA_NAME: _ZN26Inter_MonitorNoticeMessage12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED6AC / 0x84F9B34 / 0x84F9B64
    Inter_MonitorNoticeMessage();
    virtual ~Inter_MonitorNoticeMessage();
};

class Inter_MonitorForbidChat : public ISigDispatcher {
public:
    // IDA_EA: 0x84D0466
    // IDA_NAME: _ZN23Inter_MonitorForbidChat12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED700 / 0x84F9AE6 / 0x84F9B16
    Inter_MonitorForbidChat();
    virtual ~Inter_MonitorForbidChat();
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorNoticeMessage) == 0x04u,
              "Inter_MonitorNoticeMessage must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorForbidChat) == 0x04u,
              "Inter_MonitorForbidChat must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_MONITOR_NOTICE_CONTROL_H_
