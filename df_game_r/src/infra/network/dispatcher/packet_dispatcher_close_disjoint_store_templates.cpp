#include "old_game/progression/career/packet_dispatcher_close_disjoint_store.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x825401C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x825416C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254176
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CB74
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CBA4
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D8CA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598862
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25stCloseDisjointStoreParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCloseDisjointStoreParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 stCloseDisjointStoreParam,
                                 static_cast<ch_state>(3)>;
