#include "old_game/progression/career/packet_dispatcher_disjoint_store.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x82549C2
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8254B1E
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254B28
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CCFA
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CD2A
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D8E8
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x85987BA
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CREATE_DISJOINT_STORE26stCreateDisjointStoreParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,stCreateDisjointStoreParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,
                                 stCreateDisjointStoreParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x825420A
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x825435A
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254364
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CBC2
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CBF2
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D8D0
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598846
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_ENTER_DISJOINT_STORE25stEnterDisjointStoreParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,stEnterDisjointStoreParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,
                                 stEnterDisjointStoreParam,
                                 static_cast<ch_state>(3)>;
