#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_OBJECT_BRING_UP_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_OBJECT_BRING_UP_H_

#include <stdint.h>

#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA request type: MSG_BASE remains 13 bytes; the compiler's natural
// uint16_t alignment places material_slot at +0x0e, exactly as the old
// Dispatcher_ObjectBringUp reader addresses it.
struct MSG_OBJECT_BRING_UP : public MSG_BASE {
    uint16_t material_slot;
};

struct stObjectBringUp : public ParamBase {
    int error_code;
    uint16_t material_slot;
    uint8_t material_total;
    uint8_t reward_count;
    std::vector<int> reward_values;

    // IDA_EA: 0x8231AE6
    // IDA_NAME: _ZN15stObjectBringUpC2Ev
    // DEMANGLED: stObjectBringUp::stObjectBringUp(void)
    stObjectBringUp();

    // IDA_EA: 0x8246902 / 0x8246964
    // IDA_NAME: _ZN15stObjectBringUpD2Ev / _ZN15stObjectBringUpD0Ev
    // DEMANGLED: stObjectBringUp::~stObjectBringUp()
    virtual ~stObjectBringUp();
};

static_assert(sizeof(MSG_OBJECT_BRING_UP) == 0x10u,
              "CMD469 request must retain the 16-byte old-stack ABI");
#if defined(__i386__)
static_assert(sizeof(stObjectBringUp) == 0x18u,
              "CMD469 parameter must retain ParamBase+fields+vector ABI");
#endif

// CMD469.  The concrete handler is restored without mutating PacketDispatcher's
// 606-entry construction table: the root can only be activated as its complete
// original transaction.
class Dispatcher_ObjectBringUp
    : public IPacketDispatcher<MSG_OBJECT_BRING_UP,
                               stObjectBringUp,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597B5E
    // IDA_NAME: _ZN24Dispatcher_ObjectBringUpC2Ev
    // DEMANGLED: Dispatcher_ObjectBringUp::Dispatcher_ObjectBringUp(void)
    Dispatcher_ObjectBringUp();

    // IDA_EA: 0x824426C / 0x824429C
    // IDA_NAME: _ZN24Dispatcher_ObjectBringUpD2Ev /
    //           _ZN24Dispatcher_ObjectBringUpD0Ev
    // DEMANGLED: Dispatcher_ObjectBringUp::~Dispatcher_ObjectBringUp()
    virtual ~Dispatcher_ObjectBringUp();

    // IDA_EA: 0x81E147E / 0x81E14D4 / 0x81E1704
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E16CE
    // IDA_NAME: _ZN24Dispatcher_ObjectBringUp11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ObjectBringUp::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ObjectBringUp) == 0x04u,
              "CMD469 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_OBJECT_BRING_UP_H_
