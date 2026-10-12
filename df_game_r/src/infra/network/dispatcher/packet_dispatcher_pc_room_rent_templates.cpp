#include "old_game/dungeon/room/packet_dispatcher_pc_room_rent.h"

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8249AA8
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8249C04
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8249C0E
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B314
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B344
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D5AC
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x85990CE
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_RENT_ITEM15stRentItemParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_RENT_ITEM,stRentItemParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_RENT_ITEM,
                                 stRentItemParam,
                                 static_cast<ch_state>(0)>;
