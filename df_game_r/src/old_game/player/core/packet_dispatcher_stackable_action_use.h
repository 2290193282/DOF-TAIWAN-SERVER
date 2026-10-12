#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_STACKABLE_ACTION_USE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_STACKABLE_ACTION_USE_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 99_misc_global/MSG_STACKABLE_ACTION_USE.
struct __attribute__((packed)) MSG_STACKABLE_ACTION_USE : MSG_BASE {
    MSG_STACKABLE_ACTION_USE();

    int16_t slot;             // +0x0d
    uint8_t item_space;       // +0x0f
    uint32_t inventory_sum;   // +0x10
    uint8_t action_type;      // +0x14
    uint32_t arg3;            // +0x15
    uint32_t arg4;            // +0x19
};

// IDA unit: 99_misc_global/stStackableActionUse.
struct stStackableActionUse : ParamBase {
    stStackableActionUse();
    virtual ~stStackableActionUse();

    int result_code;  // +0x04
    int16_t slot;     // +0x08
    uint8_t item_space; // +0x0a
    uint8_t raw_0b;
};

// IDA unit: 13_database_dispatch/Dispatcher_StackableActionUse.
class Dispatcher_StackableActionUse
    : public IPacketDispatcher<MSG_STACKABLE_ACTION_USE,
                               stStackableActionUse,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_StackableActionUse();
    virtual ~Dispatcher_StackableActionUse();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(MSG_STACKABLE_ACTION_USE) == 0x1du,
              "CMD516 request must retain its 0x1d-byte i386 ABI");
static_assert(sizeof(stStackableActionUse) == 0x0cu,
              "CMD516 response must retain its 0x0c-byte i386 ABI");
static_assert(sizeof(Dispatcher_StackableActionUse) == 0x04u,
              "CMD516 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_STACKABLE_ACTION_USE_H_
