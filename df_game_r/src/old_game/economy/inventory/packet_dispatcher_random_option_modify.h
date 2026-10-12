#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_RANDOM_OPTION_MODIFY_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_RANDOM_OPTION_MODIFY_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA units: 99_misc_global/MSG_CHANGE_RANDOM_OPTION and
// 99_misc_global/MSG_RESET_RANDOM_OPTION.  CMD435 keeps its byte selector;
// the native CMD453 sender writes all three request values as uint16.
// Both messages leave +0x0d untouched after MSG_BASE::MSG_BASE().
#pragma pack(push, 1)
struct MSG_CHANGE_RANDOM_OPTION : public MSG_BASE {
    MSG_CHANGE_RANDOM_OPTION();

    uint8_t reserved_0d;
    uint16_t item_slot;
    uint16_t payment_slot;
    uint8_t option_number;
};

struct MSG_RESET_RANDOM_OPTION : public MSG_BASE {
    MSG_RESET_RANDOM_OPTION();

    uint8_t reserved_0d;
    uint16_t item_slot;
    uint16_t reset_item_slot;
    uint16_t option_number;
};
#pragma pack(pop)

static_assert(sizeof(MSG_CHANGE_RANDOM_OPTION) == 0x13u,
              "CMD438 request must retain IDA 19-byte extent");
static_assert(offsetof(MSG_CHANGE_RANDOM_OPTION, item_slot) == 0x0eu,
              "CMD438 target slot must retain IDA +0x0e");
static_assert(offsetof(MSG_CHANGE_RANDOM_OPTION, payment_slot) == 0x10u,
              "CMD438 payment slot must retain IDA +0x10");
static_assert(offsetof(MSG_CHANGE_RANDOM_OPTION, option_number) == 0x12u,
              "CMD438 selected option must retain IDA +0x12");
static_assert(sizeof(MSG_RESET_RANDOM_OPTION) == 0x14u,
              "CMD453 request must retain native 20-byte extent");
static_assert(offsetof(MSG_RESET_RANDOM_OPTION, item_slot) == 0x0eu,
              "CMD453 target slot must retain +0x0e");
static_assert(offsetof(MSG_RESET_RANDOM_OPTION, reset_item_slot) == 0x10u,
              "CMD453 reset material slot must retain +0x10");
static_assert(offsetof(MSG_RESET_RANDOM_OPTION, option_number) == 0x12u,
              "CMD453 selected option must retain +0x12");

// PacketDispatcher slot 438.  Its original template dispatches the packed
// request above and does the PVF-driven changed-option transaction in
// process().
class Dispatcher_ChangeRandomOption
    : public IPacketDispatcher<MSG_CHANGE_RANDOM_OPTION,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859794A
    // IDA_NAME: _ZN29Dispatcher_ChangeRandomOptionC2Ev
    Dispatcher_ChangeRandomOption();

    // IDA_EA: 0x8244836 / 0x8244866
    // IDA_NAME: _ZN29Dispatcher_ChangeRandomOptionD2Ev / D0Ev
    virtual ~Dispatcher_ChangeRandomOption();

    // IDA_EA: 0x81DC8DE / 0x81DC9BA
    // IDA_NAME: _ZN29Dispatcher_ChangeRandomOption4readER9PacketBufR8MSG_BASE
    // IDA_NAME: _ZN29Dispatcher_ChangeRandomOption7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// PacketDispatcher slot 453.  This consumes a permitted reset material and
// restores the selected normal/pending random-option value through the same
// original random stream family as CMD438.
class DispatcherResetRandomOption
    : public IPacketDispatcher<MSG_RESET_RANDOM_OPTION,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597966
    // IDA_NAME: _ZN27DispatcherResetRandomOptionC2Ev
    DispatcherResetRandomOption();

    // IDA_EA: 0x82447E8 / 0x8244818
    // IDA_NAME: _ZN27DispatcherResetRandomOptionD2Ev / D0Ev
    virtual ~DispatcherResetRandomOption();

    // IDA_EA: 0x81DD1E2 / 0x81DD2BE
    // IDA_NAME: _ZN27DispatcherResetRandomOption4readER9PacketBufR8MSG_BASE
    // IDA_NAME: _ZN27DispatcherResetRandomOption7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeRandomOption) == 0x04u,
              "CMD438 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(DispatcherResetRandomOption) == 0x04u,
              "CMD456 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_RANDOM_OPTION_MODIFY_H_
