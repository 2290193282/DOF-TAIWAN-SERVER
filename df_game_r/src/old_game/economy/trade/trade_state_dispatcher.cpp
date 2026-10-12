#include "old_game/economy/trade/trade_state_dispatcher.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x85955D4
// IDA_NAME: _ZN24DisPatcher_SetTradeStateC2Ev
// DEMANGLED: DisPatcher_SetTradeState::DisPatcher_SetTradeState(void)
DisPatcher_SetTradeState::DisPatcher_SetTradeState() {}

// IDA_EA: 0x8233164
// IDA_NAME: _ZN24DisPatcher_SetTradeStateD2Ev
// DEMANGLED: DisPatcher_SetTradeState::~DisPatcher_SetTradeState()
// IDA_EA: 0x8233194
// IDA_NAME: _ZN24DisPatcher_SetTradeStateD0Ev
// DEMANGLED: DisPatcher_SetTradeState::~DisPatcher_SetTradeState()
DisPatcher_SetTradeState::~DisPatcher_SetTradeState() {}

// IDA_EA: 0x81F6232
// IDA_NAME: _ZN24DisPatcher_SetTradeState12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetTradeState::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetTradeState::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    char state = 0;
    if (packet.get_byte(state) != 1) {
        return LineFunc(
            33510,
            "virtual int DisPatcher_SetTradeState::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CTradeSpace* const trade_space =
        G_CGameManager()->GetTradeSpace(user->GetTradeSpace());
    if (!trade_space) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 1);
        return 0;
    }

    if (trade_space->is_empty()) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            "virtual int DisPatcher_SetTradeState::dispatch_sig(CUser*, PacketBuf&)",
            33521,
            "Trade Space[%d] is Empty User[%d]",
            trade_space->get_index(),
            static_cast<unsigned short>(user->get_unique_id()));
        G_CGameManager()->PutTradeSpace(trade_space);
        return 0;
    }

    if (trade_space->check_trade_possibility() == 1) {
        trade_space->change_trade_state(user, state);
    } else {
        trade_space->cancel_trade();
        G_CGameManager()->PutTradeSpace(trade_space);
    }
    return 0;
}

// IDA_EA: 0x8233150
// IDA_NAME: _ZN24DisPatcher_SetTradeState4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetTradeState::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetTradeState::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823315A
// IDA_NAME: _ZN24DisPatcher_SetTradeState7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetTradeState::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetTradeState::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
