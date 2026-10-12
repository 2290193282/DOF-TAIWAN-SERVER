#include "old_game/player/core/packet_dispatcher_mercenary_return.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// This semantic C/userinfo owner emits the original state-0 generic triplet.
// It is intentionally not registered in the root PacketDispatcher.
// IDA_EA: 0x824D69C
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824D7EC
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824D7F6
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825BB9C
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BBCC
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D642
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598E2E
// IDA_NAME: _ZN17IPacketDispatcherI18MsgMercenaryReturn25SendMercenaryReturnResultL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MsgMercenaryReturn,SendMercenaryReturnResult,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MsgMercenaryReturn,
                                 SendMercenaryReturnResult,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x824D624
// IDA_NAME: _ZN25SendMercenaryReturnResultC2Ev
// DEMANGLED: SendMercenaryReturnResult::SendMercenaryReturnResult(void)
SendMercenaryReturnResult::SendMercenaryReturnResult()
    : ParamBase(),
      reward_result() {}

// IDA_EA: 0x824D64E
// IDA_NAME: _ZN25SendMercenaryReturnResultD2Ev
// DEMANGLED: SendMercenaryReturnResult::~SendMercenaryReturnResult()
// IDA_EA: 0x824D67E
// IDA_NAME: _ZN25SendMercenaryReturnResultD0Ev
// DEMANGLED: SendMercenaryReturnResult::~SendMercenaryReturnResult()
SendMercenaryReturnResult::~SendMercenaryReturnResult() {}

// IDA_EA: 0x859799E
// IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnC2Ev
// DEMANGLED: Dispatcher_Mercenary_Return::Dispatcher_Mercenary_Return(void)
Dispatcher_Mercenary_Return::Dispatcher_Mercenary_Return()
    : IPacketDispatcher<MsgMercenaryReturn,
                        SendMercenaryReturnResult,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824474C
// IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnD2Ev
// DEMANGLED: Dispatcher_Mercenary_Return::~Dispatcher_Mercenary_Return()
// IDA_EA: 0x824477C
// IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnD0Ev
// DEMANGLED: Dispatcher_Mercenary_Return::~Dispatcher_Mercenary_Return()
Dispatcher_Mercenary_Return::~Dispatcher_Mercenary_Return() {}

// IDA_EA: 0x81DD904
// IDA_NAME: _ZN27Dispatcher_Mercenary_Return4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Mercenary_Return::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Mercenary_Return::read(PacketBuf& packet, MSG_BASE& message) {
    MsgMercenaryReturn& input = static_cast<MsgMercenaryReturn&>(message);
    if (packet.get_byte(input.reason) != 1) {
        return LineFunc(20104,
                        "virtual int Dispatcher_Mercenary_Return::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(input.charac_no) == 1) {
        return 0;
    }
    return LineFunc(20106,
                    "virtual int Dispatcher_Mercenary_Return::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81DD99C
// IDA_NAME: _ZN27Dispatcher_Mercenary_Return7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Return::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Mercenary_Return::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return -1;
    }

    const MsgMercenaryReturn& input =
        static_cast<const MsgMercenaryReturn&>(message);
    SendMercenaryReturnResult& result =
        static_cast<SendMercenaryReturnResult&>(param);
    result.return_result = static_cast<uint8_t>(
        user->Return_Mercenary_Charac(input.charac_no, result.reward_result));
    result.reason = input.reason;
    result.charac_no = input.charac_no;
    return 0;
}

// IDA_EA: 0x81DDA1C
// IDA_NAME: _ZN27Dispatcher_Mercenary_Return4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Return::send(CUser *,ParamBase &)
void Dispatcher_Mercenary_Return::send(CUser* user, ParamBase& param) {
    const SendMercenaryReturnResult& result =
        static_cast<const SendMercenaryReturnResult&>(param);
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_MERCENARY_RETURN);
    packet.put_byte(result.return_result != 0);
    packet.put_byte(result.reason);
    packet.put_int(result.charac_no);
    packet.put_int(result.reward_result.first_dword);
    packet.put_int(result.reward_result.second_dword);
    packet.put_byte(result.reward_result.success_flag);
    packet.finalize(true);
    user->Send(packet);
}
