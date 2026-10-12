#include "old_game/progression/career/packet_dispatcher_disjoint_machine.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x82545E6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8254736
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254740
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CC5E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CC8E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D8DC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x85987F2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE28stRepairDisjointMachineParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRepairDisjointMachineParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 stRepairDisjointMachineParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x82543F8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8254548
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254552
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CC10
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CC40
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D8D6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x859882A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stUpgradeDisjointMachineParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stUpgradeDisjointMachineParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 stUpgradeDisjointMachineParam,
                                 static_cast<ch_state>(3)>;
