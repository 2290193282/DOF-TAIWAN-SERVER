#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_TELEPORT_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_TELEPORT_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_Teleport.  The original request
// starts after the 0x0d-byte MSG_BASE header.  Byte +0x0d is untouched by the
// decoder and therefore remains a physical protocol byte rather than a
// manufactured semantic field.
#pragma pack(push, 1)
struct MSG_TELEPORT : public MSG_BASE {
    uint8_t raw_0d;
    uint16_t stackable_slot;       // IDA +0x0e
    uint32_t request_item_token;   // IDA +0x10
    uint8_t item_space;            // IDA +0x14
    uint8_t target_village;        // IDA +0x15

};
#pragma pack(pop)

struct stTeleportParam : public ParamBase {
    stTeleportParam();
    virtual ~stTeleportParam();

    int result;                    // IDA +0x04
    uint16_t stackable_slot;       // IDA +0x08
    uint16_t padding_0a;
    uint32_t request_item_token;   // IDA +0x0c
    uint8_t item_space;            // IDA +0x10
    uint8_t target_village;        // IDA +0x11
    uint16_t padding_12;
    uint32_t consumed_item_id;     // IDA +0x14
};

// Source-complete derived provider only.  PacketDispatcher's 606-slot
// registry remains separately owned and is deliberately not amended here.
class Dispatcher_Teleport
    : public IPacketDispatcher<MSG_TELEPORT,
                               stTeleportParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_Teleport();
    virtual ~Dispatcher_Teleport();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(offsetof(MSG_TELEPORT, stackable_slot) == 0x0eu,
              "CMD241 stackable slot must stay at IDA +0x0e");
static_assert(offsetof(MSG_TELEPORT, request_item_token) == 0x10u,
              "CMD241 request token must stay at IDA +0x10");
static_assert(offsetof(MSG_TELEPORT, item_space) == 0x14u,
              "CMD241 item-space must stay at IDA +0x14");
static_assert(offsetof(MSG_TELEPORT, target_village) == 0x15u,
              "CMD241 village must stay at IDA +0x15");
static_assert(sizeof(MSG_TELEPORT) == 0x16u,
              "CMD241 request must retain its old 0x16-byte extent");
static_assert(offsetof(stTeleportParam, result) == 0x04u,
              "CMD241 result must stay at IDA +0x04");
static_assert(offsetof(stTeleportParam, stackable_slot) == 0x08u,
              "CMD241 slot must stay at IDA +0x08");
static_assert(offsetof(stTeleportParam, consumed_item_id) == 0x14u,
              "CMD241 consumed item id must stay at IDA +0x14");
static_assert(sizeof(stTeleportParam) == 0x18u,
              "CMD241 parameter must retain its old 0x18-byte extent");
static_assert(sizeof(Dispatcher_Teleport) == 0x04u,
              "CMD241 dispatcher must retain its i386 vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_TELEPORT_DISPATCHER_H_
