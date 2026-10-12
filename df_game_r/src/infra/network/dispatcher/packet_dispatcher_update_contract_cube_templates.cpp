#include "old_game/player/core/packet_dispatcher_update_contract_cube.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8261132
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x826128E
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8261298
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8262398
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82623C8
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8262618
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x859921E
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_UPDATE_CONTRACT_OF_CUBE_INFO26stUpdateContractOfCubeInfoL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,stUpdateContractOfCubeInfo,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,
                                 stUpdateContractOfCubeInfo,
                                 static_cast<ch_state>(0)>;
