#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_STATE_DISPATCHER_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_STATE_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 10_mail_trade_auction_cash/DisPatcher_SetTradeState.
// The class has no state after the state-0 packet-dispatch base.
class DisPatcher_SetTradeState
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85955D4
    // IDA_NAME: _ZN24DisPatcher_SetTradeStateC2Ev
    // DEMANGLED: DisPatcher_SetTradeState::DisPatcher_SetTradeState(void)
    DisPatcher_SetTradeState();

    // IDA_EA: 0x8233164
    // IDA_NAME: _ZN24DisPatcher_SetTradeStateD2Ev
    // DEMANGLED: DisPatcher_SetTradeState::~DisPatcher_SetTradeState()
    // IDA_EA: 0x8233194
    // IDA_NAME: _ZN24DisPatcher_SetTradeStateD0Ev
    // DEMANGLED: DisPatcher_SetTradeState::~DisPatcher_SetTradeState()
    virtual ~DisPatcher_SetTradeState();

    // IDA_EA: 0x81F6232
    // IDA_NAME: _ZN24DisPatcher_SetTradeState12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_SetTradeState::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233150
    // IDA_NAME: _ZN24DisPatcher_SetTradeState4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_SetTradeState::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823315A
    // IDA_NAME: _ZN24DisPatcher_SetTradeState7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_SetTradeState::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetTradeState) == 0x04u,
              "CMD26 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_STATE_DISPATCHER_H_
