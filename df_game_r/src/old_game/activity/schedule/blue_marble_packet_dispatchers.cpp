#include "old_game/activity/schedule/blue_marble_packet_dispatchers.h"

#include <stdint.h>

#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"


namespace {

const char kBlueMarbleReadSignature[] =
    "virtual int Dispatcher_BlueMarble::read(PacketBuf&, MSG_BASE&)";
const char kBlueMarbleProcessSignature[] =
    "virtual int Dispatcher_BlueMarble::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x82494CA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8249626
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8249630
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B22A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B25A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D59A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8599106
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_BLUE_MARBLE12stBlueMarbleL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE,stBlueMarble,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BLUE_MARBLE,
                                 stBlueMarble,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597DC6
// IDA_NAME: _ZN21Dispatcher_BlueMarbleC2Ev
// DEMANGLED: Dispatcher_BlueMarble::Dispatcher_BlueMarble(void)
Dispatcher_BlueMarble::Dispatcher_BlueMarble() {}

// IDA_EA: 0x8243CA2
// IDA_NAME: _ZN21Dispatcher_BlueMarbleD2Ev
// DEMANGLED: Dispatcher_BlueMarble::~Dispatcher_BlueMarble()
// IDA_EA: 0x8243CD2
// IDA_NAME: _ZN21Dispatcher_BlueMarbleD0Ev
// DEMANGLED: Dispatcher_BlueMarble::~Dispatcher_BlueMarble() deleting destructor
Dispatcher_BlueMarble::~Dispatcher_BlueMarble() {}

// IDA_EA: 0x81E5F86
// IDA_NAME: _ZN21Dispatcher_BlueMarble4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BlueMarble::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BlueMarble::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_BLUE_MARBLE& request = static_cast<MSG_BLUE_MARBLE&>(message);
    if (packet.get_byte(request.action) == 1) {
        return 0;
    }
    return LineFunc(24730, kBlueMarbleReadSignature, 0, 0);
}

// IDA_EA: 0x81E612C
// IDA_NAME: _ZN21Dispatcher_BlueMarble11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_BlueMarble::check_error(CUser *,MSG_BASE &)
int Dispatcher_BlueMarble::check_error(CUser* user, MSG_BASE& message) {
    const MSG_BLUE_MARBLE& request =
        static_cast<const MSG_BLUE_MARBLE&>(message);
    if (!user) {
        return -1;
    }
    if (request.action == 6) {
        return 0;
    }
    if (user->getMoveSpace() != 1) {
        return -2;
    }
    BlueMarble* const blue_marble = user->getBlueMarble();
    if (!blue_marble) {
        return -3;
    }
    if (blue_marble->getUserSlot(user) == -1) {
        return -4;
    }
    if (user->get_state() != 11 && user->get_state() != 5) {
        return -5;
    }
    return 0;
}

// IDA_EA: 0x81E5FDC
// IDA_NAME: _ZN21Dispatcher_BlueMarble7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BlueMarble::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BlueMarble::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    const MSG_BLUE_MARBLE& request =
        static_cast<const MSG_BLUE_MARBLE&>(message);
    stBlueMarble& response = static_cast<stBlueMarble&>(param);
    const int error = check_error(user, message);
    response.dword_04 = static_cast<uint32_t>(error);
    if (error > 0) {
        return 0;
    }
    if (error < 0) {
        return LineFunc(24745, kBlueMarbleProcessSignature, error, 0);
    }

    BlueMarble* const blue_marble = user->getBlueMarble();
    if (!blue_marble) {
        return 0;
    }

    switch (request.action) {
    case 0:
        blue_marble->startBlueMarble();
        break;
    case 1:
        blue_marble->userThrowDice(user);
        break;
    case 2:
        blue_marble->enterDungeon(user);
        break;
    case 3:
        blue_marble->giveupDungeon(user);
        break;
    case 4:
        blue_marble->clearDungeon(user);
        break;
    case 5:
        blue_marble->exitDungeon(user);
        break;
    case 6:
        blue_marble->leaveUser(user);
        break;
    case 7:
        blue_marble->giveBonusItem(user);
        break;
    default:
        return 0;
    }
    return 0;
}

// IDA_EA: 0x81E61F4
// IDA_NAME: _ZN21Dispatcher_BlueMarble4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_BlueMarble::send(CUser *,ParamBase &)
void Dispatcher_BlueMarble::send(CUser* user, ParamBase& param) {
    stBlueMarble& response = static_cast<stBlueMarble&>(param);
    if (response.dword_04 != 0u) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BLUE_MARBLE,
            static_cast<unsigned char>(response.dword_04));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_BLUE_MARBLE);
}
