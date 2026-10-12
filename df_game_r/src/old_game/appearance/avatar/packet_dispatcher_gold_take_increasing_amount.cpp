#include "old_game/appearance/avatar/packet_dispatcher_gold_take_increasing_amount.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_GoldTakeIncreasingAmount::read(PacketBuf&, MSG_BASE&)";
const char kCheckErrorSignature[] =
    "int Dispatcher_GoldTakeIncreasingAmount::check_error(CUser*, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x82315D8
// IDA_NAME: _ZN26stGoldTakeIncreasingAmountC2Ev
// DEMANGLED: stGoldTakeIncreasingAmount::stGoldTakeIncreasingAmount(void)
stGoldTakeIncreasingAmount::stGoldTakeIncreasingAmount()
    : ParamBase(), item_id(0), response_flag(0) {}

// IDA_EA: 0x8246FEE
// IDA_NAME: _ZN26stGoldTakeIncreasingAmountD2Ev
// IDA_EA: 0x824701E
// IDA_NAME: _ZN26stGoldTakeIncreasingAmountD0Ev
// DEMANGLED: stGoldTakeIncreasingAmount::~stGoldTakeIncreasingAmount()
stGoldTakeIncreasingAmount::~stGoldTakeIncreasingAmount() {}

// IDA_EA: 0x81D554E
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GoldTakeIncreasingAmount::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    MSG_GOLD_TAKE_INCREASING_AMOUNT& request =
        static_cast<MSG_GOLD_TAKE_INCREASING_AMOUNT&>(message);
    if (packet.get_int(request.item_id) != 1) {
        return LineFunc(14550, kReadSignature, 0, 0);
    }
    return packet.get_byte(request.activation_flag) == 1
               ? 0
               : LineFunc(14552, kReadSignature, 0, 0);
}

// IDA_EA: 0x81D55A4
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GoldTakeIncreasingAmount::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    const int check = check_error(user, message);
    if (check > 0) {
        return check;
    }
    if (check < 0) {
        return -1;
    }

    const MSG_GOLD_TAKE_INCREASING_AMOUNT& request =
        static_cast<const MSG_GOLD_TAKE_INCREASING_AMOUNT&>(message);
    stGoldTakeIncreasingAmount& result =
        static_cast<stGoldTakeIncreasingAmount&>(param);
    result.item_id = request.item_id;
    if (result.item_id != 63114 || request.activation_flag != 1) {
        return 21;
    }

    user_creature::CCreatureMgr* const creatures =
        user->getCurCharacInvenW()->GetCreatureMgrW();
    // The original writes zero for a nonzero SetOverSkillTime result and one
    // for zero.  Preserve the protocol byte polarity exactly.
    result.response_flag = creatures->SetOverSkillTime(result.item_id) ? 0 : 1;
    return 0;
}

// IDA_EA: 0x81D5656
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::check_error(CUser *,MSG_BASE &)
int Dispatcher_GoldTakeIncreasingAmount::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return LineFunc(14595, kCheckErrorSignature, 0, 0);
    }
    return user->get_state() == 5 ? 0 : -1;
}

// IDA_EA: 0x81D56AC
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmount4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::send(CUser *,ParamBase &)
void Dispatcher_GoldTakeIncreasingAmount::send(CUser* user, ParamBase& param) {
    const stGoldTakeIncreasingAmount& result =
        static_cast<const stGoldTakeIncreasingAmount&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GOLD_TAKE_INCREASING_AMOUNT);
    packet.put_byte(result.response_flag);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8244FD4
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmountD2Ev
// IDA_EA: 0x8245004
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmountD0Ev
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::~Dispatcher_GoldTakeIncreasingAmount()
Dispatcher_GoldTakeIncreasingAmount::~Dispatcher_GoldTakeIncreasingAmount() {}

// IDA_EA: 0x859763A
// IDA_NAME: _ZN35Dispatcher_GoldTakeIncreasingAmountC2Ev
// DEMANGLED: Dispatcher_GoldTakeIncreasingAmount::Dispatcher_GoldTakeIncreasingAmount(void)
Dispatcher_GoldTakeIncreasingAmount::Dispatcher_GoldTakeIncreasingAmount()
    : IPacketDispatcher<MSG_GOLD_TAKE_INCREASING_AMOUNT,
                        stGoldTakeIncreasingAmount,
                        static_cast<ch_state>(0)>() {}
