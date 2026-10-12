#include "old_game/progression/quest/packet_dispatcher_req_urgent_quest.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824C568
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824C6B8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824C6C2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B92C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B95C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D612
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598F2A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ReqResultL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ReqResult,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 ReqResult,
                                 static_cast<ch_state>(3)>;
