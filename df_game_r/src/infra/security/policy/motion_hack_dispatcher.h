#ifndef OLDSRV_SECURITY_MOTION_HACK_DISPATCHER_H_
#define OLDSRV_SECURITY_MOTION_HACK_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 358 / ENUM_CMDPACKET_DUNGEON_MOTION.  Raw
// IPacketDispatcher<MSG_MOTION_HACK,...>::dispatch_template @0x82663DE
// places precisely 0x16 bytes on its stack and calls no named MSG constructor.
// Keep this packed overlay free of a user-declared C2 and of field initializers.
#pragma pack(push, 1)
struct MSG_MOTION_HACK : MSG_BASE {
    uint8_t enabled;
    uint32_t value1;
    uint32_t value2;
};
#pragma pack(pop)

static_assert(sizeof(MSG_MOTION_HACK) == 0x16u,
              "CMD358 MSG_MOTION_HACK must retain the raw 0x16-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_MOTION_HACK, enabled) == 0x0du &&
                  offsetof(MSG_MOTION_HACK, value1) == 0x0eu &&
                  offsetof(MSG_MOTION_HACK, value2) == 0x12u,
              "CMD358 MSG_MOTION_HACK offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The old PacketDispatcher constructor allocates four i386 bytes at slot 358
// for this state-0 typed dispatcher.  The live 606-slot transaction is still
// unavailable, so this provider is deliberately not registered.
class Dispatcher_MotionHack
    : public IPacketDispatcher<MSG_MOTION_HACK,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859753E
    // IDA_NAME: _ZN21Dispatcher_MotionHackC2Ev
    // DEMANGLED: Dispatcher_MotionHack::Dispatcher_MotionHack(void)
    Dispatcher_MotionHack();

    // IDA_EA: 0x82656C6
    // IDA_NAME: _ZN21Dispatcher_MotionHackD2Ev
    // DEMANGLED: Dispatcher_MotionHack::~Dispatcher_MotionHack()
    // IDA_EA: 0x82656F6
    // IDA_NAME: _ZN21Dispatcher_MotionHackD0Ev
    // DEMANGLED: Dispatcher_MotionHack::~Dispatcher_MotionHack()
    virtual ~Dispatcher_MotionHack();

    // IDA_EA: 0x8264574
    // IDA_NAME: _ZN21Dispatcher_MotionHack4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MotionHack::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x826464A
    // IDA_NAME: _ZN21Dispatcher_MotionHack7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MotionHack::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MotionHack) == 0x04u,
              "CMD358 dispatcher must retain the raw four-byte i386 base");
#endif

#endif  // OLDSRV_SECURITY_MOTION_HACK_DISPATCHER_H_
