#include "old_game/progression/quest/packet_dispatcher_debug_clear_quest.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824C34E
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824C4CC
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824C4D6
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B8DE / 0x825B90E
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EED2Ev / D0Ev
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D60C
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598F46
// IDA_NAME: _ZN17IPacketDispatcherI18MsgClearQuestIndex24ReqDebugClearQuestResultL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MsgClearQuestIndex,ReqDebugClearQuestResult,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MsgClearQuestIndex,
                                 ReqDebugClearQuestResult,
                                 static_cast<ch_state>(0)>;
