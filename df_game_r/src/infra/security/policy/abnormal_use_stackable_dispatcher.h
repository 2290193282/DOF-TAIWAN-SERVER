#ifndef OLDSRV_SECURITY_ABNORMAL_USE_STACKABLE_DISPATCHER_H_
#define OLDSRV_SECURITY_ABNORMAL_USE_STACKABLE_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA's CMD437 template creates a 24-byte local MSG_ABNORMAL_USE_STACKABLE.
// The 13-byte C2S base naturally aligns the two values to offsets 16 and 20.
struct MSG_ABNORMAL_USE_STACKABLE : MSG_BASE {
    uint32_t first_value;
    uint32_t second_value;
};

static_assert(sizeof(MSG_ABNORMAL_USE_STACKABLE) == 0x18u,
              "CMD437 message must retain its 24-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_ABNORMAL_USE_STACKABLE, first_value) == 0x10u &&
                  offsetof(MSG_ABNORMAL_USE_STACKABLE, second_value) == 0x14u,
              "CMD437 message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class AbnormalUseStackable : public ParamBase {
public:
    // IDA_EA: 0x8265448
    // IDA_NAME: _ZN20AbnormalUseStackableC2Ev
    // DEMANGLED: AbnormalUseStackable::AbnormalUseStackable(void)
    AbnormalUseStackable();

    // IDA_EA: 0x826589A
    // IDA_NAME: _ZN20AbnormalUseStackableD2Ev
    // DEMANGLED: AbnormalUseStackable::~AbnormalUseStackable()
    // IDA_EA: 0x82658CA
    // IDA_NAME: _ZN20AbnormalUseStackableD0Ev
    // DEMANGLED: AbnormalUseStackable::~AbnormalUseStackable()
    virtual ~AbnormalUseStackable();

    uint8_t raw_04;
};

class Dispatcher_AbnormalUseStackable
    : public IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,
                               AbnormalUseStackable,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859792E
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableC2Ev
    // DEMANGLED: Dispatcher_AbnormalUseStackable::Dispatcher_AbnormalUseStackable(void)
    Dispatcher_AbnormalUseStackable();

    // IDA_EA: 0x82655DC
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableD2Ev
    // DEMANGLED: Dispatcher_AbnormalUseStackable::~Dispatcher_AbnormalUseStackable()
    // IDA_EA: 0x826560C
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableD0Ev
    // DEMANGLED: Dispatcher_AbnormalUseStackable::~Dispatcher_AbnormalUseStackable()
    virtual ~Dispatcher_AbnormalUseStackable();

    // IDA_EA: 0x826491C
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AbnormalUseStackable::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82649AE
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AbnormalUseStackable::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8264A30
    // IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_AbnormalUseStackable::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(AbnormalUseStackable) == 0x08u,
              "CMD437 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(Dispatcher_AbnormalUseStackable) == 0x04u,
              "CMD437 dispatcher must retain its four-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(AbnormalUseStackable, raw_04) == 0x04u,
              "CMD437 parameter result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_SECURITY_ABNORMAL_USE_STACKABLE_DISPATCHER_H_
