#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_GOLD_TAKE_INCREASING_AMOUNT_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_GOLD_TAKE_INCREASING_AMOUNT_H_

#include <stdint.h>

#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The restored state-0 template owns the request storage.  The native client
// appends an activation byte after the creature item id (sender 0x10773A0).
#pragma pack(push, 1)
struct MSG_GOLD_TAKE_INCREASING_AMOUNT : public MSG_BASE {
    int32_t item_id;          // +0x0d
    uint8_t activation_flag;  // +0x11; native client sends 1
};
#pragma pack(pop)

static_assert(sizeof(MSG_GOLD_TAKE_INCREASING_AMOUNT) == 0x12u,
              "CMD368 request must retain its native 18-byte extent");
static_assert(offsetof(MSG_GOLD_TAKE_INCREASING_AMOUNT, item_id) == 0x0du,
              "MSG_GOLD_TAKE_INCREASING_AMOUNT item offset changed");
static_assert(offsetof(MSG_GOLD_TAKE_INCREASING_AMOUNT, activation_flag) ==
                  0x11u,
              "CMD368 activation flag offset changed");

// IDA unit: 99_misc_global/stGoldTakeIncreasingAmount.
struct stGoldTakeIncreasingAmount : public ParamBase {
    // IDA_EA: 0x82315D8
    // IDA_NAME: _ZN26stGoldTakeIncreasingAmountC2Ev
    // DEMANGLED: stGoldTakeIncreasingAmount::stGoldTakeIncreasingAmount(void)
    stGoldTakeIncreasingAmount();

    // IDA_EA: 0x8246FEE / 0x824701E
    // IDA_NAME: _ZN26stGoldTakeIncreasingAmountD2Ev / D0Ev
    // DEMANGLED: stGoldTakeIncreasingAmount::~stGoldTakeIncreasingAmount()
    virtual ~stGoldTakeIncreasingAmount();

    int32_t item_id;       // +0x04
    uint8_t response_flag; // +0x08
};

#if defined(__i386__)
static_assert(sizeof(stGoldTakeIncreasingAmount) == 0x0cu,
              "stGoldTakeIncreasingAmount must retain its i386 ABI size");
#endif

// The generic members are emitted once by the A-owned template translation
// unit.  These declarations must precede the derived class: otherwise its
// vtable formation instantiates primary-template COMDAT members before the
// exact old providers at 0x8251158/0x8251162/0x825D746 are seen.
template <>
IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                  stGoldTakeIncreasingAmount,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                  stGoldTakeIncreasingAmount,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                      PacketBuf&);
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                 PacketBuf&);
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                                PacketBuf&);
template <>
void IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                       stGoldTakeIncreasingAmount,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA unit: 13_database_dispatch/Dispatcher_GoldTakeIncreasingAmount.
struct Dispatcher_GoldTakeIncreasingAmount
    : public IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                               stGoldTakeIncreasingAmount,
                               static_cast<ch_state>(0)> {
    // IDA_EA: 0x859763A
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmountC2Ev
    // DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::Dispatcher_GoldTakeIncreasingAmount(void)
    Dispatcher_GoldTakeIncreasingAmount();

    // IDA_EA: 0x8244FD4 / 0x8245004
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmountD2Ev / D0Ev
    // DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::~Dispatcher_GoldTakeIncreasingAmount()
    virtual ~Dispatcher_GoldTakeIncreasingAmount();

    // IDA_EA: 0x81D554E
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D55A4
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D56AC
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D5656
    // IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GoldTakeIncreasingAmount) == 0x04u,
              "Dispatcher_GoldTakeIncreasingAmount must retain its i386 ABI size");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_GOLD_TAKE_INCREASING_AMOUNT_H_
