#include "old_game/world/region/packet_dispatcher_set_pos_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"

// This inclusion is translation-unit local.  The sole explicit instantiation
// below owns the seven exact old state-0 template symbols for CMD37; no local
// copy of the common dispatcher template is introduced.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int DisPatcher_SetPos::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int DisPatcher_SetPos::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// The exact old state-0 template is emitted by the shared canonical template
// body.  The markers stay beside its sole explicit instantiation owner.
// IDA_EA: 0x8258576
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82586C6
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82586D0
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8258762
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8236834
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8236864
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x859819A
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USER_SET_POS9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_SET_POS,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_USER_SET_POS,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x85952A0
// IDA_NAME: _ZN17DisPatcher_SetPosC2Ev
// DEMANGLED: DisPatcher_SetPos::DisPatcher_SetPos(void)
DisPatcher_SetPos::DisPatcher_SetPos() {}

// IDA_EA: 0x8232680
// IDA_NAME: _ZN17DisPatcher_SetPosD2Ev
// DEMANGLED: DisPatcher_SetPos::~DisPatcher_SetPos()
// IDA_EA: 0x82326B0
// IDA_NAME: _ZN17DisPatcher_SetPosD0Ev
// DEMANGLED: DisPatcher_SetPos::~DisPatcher_SetPos()
DisPatcher_SetPos::~DisPatcher_SetPos() {}

// IDA_EA: 0x81C97A6
// IDA_NAME: _ZN17DisPatcher_SetPos4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPos::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPos::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USER_SET_POS& input = static_cast<MSG_USER_SET_POS&>(message);
    if (packet.get_short(input.position_x) != 1) {
        return LineFunc(7330, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.position_y) != 1) {
        return LineFunc(7332, kReadSignature, 0, 0);
    }
    if (packet.get_byte(input.direction) != 1) {
        return LineFunc(7334, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.position_context) == 1) {
        return 0;
    }
    return LineFunc(7336, kReadSignature, 0, 0);
}

// IDA_EA: 0x81C98C8
// IDA_NAME: _ZN17DisPatcher_SetPos7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPos::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPos::process(CUser* user,
                               MSG_BASE& message,
                               ParamBase& param) {
    (void)param;
    const int error = check_error(user, message);
    if (error > 0) {
        return -1;
    }
    if (error >= 0) {
        const MSG_USER_SET_POS& input =
            static_cast<const MSG_USER_SET_POS&>(message);
        G_GameWorld()->move_position(user,
                                     static_cast<int>(input.position_x),
                                     static_cast<int>(input.position_y),
                                     static_cast<int>(input.direction),
                                     input.position_context);
        return error;
    }
    return LineFunc(7352,
                    kProcessSignature,
                    0,
                    static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81C9994
// IDA_NAME: _ZN17DisPatcher_SetPos11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_SetPos::check_error(CUser *,MSG_BASE &)
int DisPatcher_SetPos::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    return user->get_state() == 3 ? 0 : 19;
}
