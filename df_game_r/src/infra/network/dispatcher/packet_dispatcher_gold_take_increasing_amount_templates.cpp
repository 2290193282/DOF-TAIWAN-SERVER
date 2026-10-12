#include "old_game/appearance/avatar/packet_dispatcher_gold_take_increasing_amount.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

typedef std::aligned_storage<sizeof(MSG_GOLD_TAKE_INCREASING_AMOUNT),
                             alignof(MSG_GOLD_TAKE_INCREASING_AMOUNT)>::type
    GoldTakeIncreasingAmountMessageStorage;

static_assert(sizeof(GoldTakeIncreasingAmountMessageStorage) == 0x12u,
              "CMD368 native request must retain its 18-byte extent");

}  // namespace

// IDA_EA: 0x8598B8E
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                  stGoldTakeIncreasingAmount,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C3D6
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EED2Ev
// IDA_EA: 0x825C406
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                  stGoldTakeIncreasingAmount,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8251008
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    GoldTakeIncreasingAmountMessageStorage message_storage;
    MSG_GOLD_TAKE_INCREASING_AMOUNT& message =
        *reinterpret_cast<MSG_GOLD_TAKE_INCREASING_AMOUNT*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stGoldTakeIncreasingAmount param;
    result = process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8251158
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8251162
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                      stGoldTakeIncreasingAmount,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_GOLD_TAKE_INCREASING_AMOUNT, PARAM_TYPE = stGoldTakeIncreasingAmount, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D746
// IDA_NAME: _ZN17IPacketDispatcherI31MSG_GOLD_TAKE_INCREASING_AMOUNT26stGoldTakeIncreasingAmountL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,stGoldTakeIncreasingAmount,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                       stGoldTakeIncreasingAmount,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
