#include "old_game/player/core/packet_dispatcher_mercenary_info.h"
#include "infra/network/packet/PacketEnum.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85979BA
// IDA_NAME: _ZN25Dispatcher_Mercenary_InfoC2Ev
// DEMANGLED: Dispatcher_Mercenary_Info::Dispatcher_Mercenary_Info(void)
Dispatcher_Mercenary_Info::Dispatcher_Mercenary_Info()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>() {}

// IDA_EA: 0x82446FE
// IDA_NAME: _ZN25Dispatcher_Mercenary_InfoD2Ev
// DEMANGLED: Dispatcher_Mercenary_Info::~Dispatcher_Mercenary_Info()
// IDA_EA: 0x824472E
// IDA_NAME: _ZN25Dispatcher_Mercenary_InfoD0Ev
// DEMANGLED: Dispatcher_Mercenary_Info::~Dispatcher_Mercenary_Info()
Dispatcher_Mercenary_Info::~Dispatcher_Mercenary_Info() {}

// IDA_EA: 0x81DDB3C
// IDA_NAME: _ZN25Dispatcher_Mercenary_Info4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Mercenary_Info::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Mercenary_Info::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81DDB46
// IDA_NAME: _ZN25Dispatcher_Mercenary_Info7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Info::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Mercenary_Info::process(CUser* user, MSG_BASE&, ParamBase&) {
    if (!user) {
        return -1;
    }
    if (user->get_state() > 1) {
        return 0;
    }
    return -1;
}

// IDA_EA: 0x81DDB7C
// IDA_NAME: _ZN25Dispatcher_Mercenary_Info4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Info::send(CUser *,ParamBase &)
void Dispatcher_Mercenary_Info::send(CUser* user, ParamBase&) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MERCENARY_INFO);
    packet.put_byte(1);
    user->SetMercenaryInfoPacket(packet);
    packet.finalize(true);
    user->Send(packet);
}
