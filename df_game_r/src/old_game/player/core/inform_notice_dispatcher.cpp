#include "old_game/player/core/inform_notice_dispatcher.h"

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_InformNotice::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_InformNotice::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x822E27A
// IDA_NAME: _ZN17MSG_INFORM_NOTICEC2Ev
// DEMANGLED: MSG_INFORM_NOTICE::MSG_INFORM_NOTICE(void)
MSG_INFORM_NOTICE::MSG_INFORM_NOTICE() : MSG_BASE(), flag_index(128u) {}

// IDA_EA: 0x8231BCA
// IDA_NAME: _ZN16InformNoticeDataC2Ev
// DEMANGLED: InformNoticeData::InformNoticeData(void)
InformNoticeData::InformNoticeData() : ParamBase(), result(0) {}

// IDA_EA: 0x8246818
// IDA_NAME: _ZN16InformNoticeDataD2Ev
// DEMANGLED: InformNoticeData::~InformNoticeData()
// IDA_EA: 0x8246848
// IDA_NAME: _ZN16InformNoticeDataD0Ev
// DEMANGLED: InformNoticeData::~InformNoticeData()
InformNoticeData::~InformNoticeData() {}

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824AE7A
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824AFD6
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824AFE0
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B5D2 / 0x825B602
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EED0Ev
// IDA_EA: 0x825D5DC
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x859905E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_INFORM_NOTICE16InformNoticeDataL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_INFORM_NOTICE,
                                 InformNoticeData,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597CE6
// IDA_NAME: _ZN23Dispatcher_InformNoticeC2Ev
// DEMANGLED: Dispatcher_InformNotice::Dispatcher_InformNotice(void)
Dispatcher_InformNotice::Dispatcher_InformNotice() {}

// IDA_EA: 0x8244098
// IDA_NAME: _ZN23Dispatcher_InformNoticeD2Ev
// DEMANGLED: Dispatcher_InformNotice::~Dispatcher_InformNotice()
// IDA_EA: 0x82440C8
// IDA_NAME: _ZN23Dispatcher_InformNoticeD0Ev
// DEMANGLED: Dispatcher_InformNotice::~Dispatcher_InformNotice()
Dispatcher_InformNotice::~Dispatcher_InformNotice() {}

// IDA_EA: 0x81E2F22
// IDA_NAME: _ZN23Dispatcher_InformNotice4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_InformNotice::read(PacketBuf &,MSG_BASE &)
int Dispatcher_InformNotice::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_INFORM_NOTICE& input = static_cast<MSG_INFORM_NOTICE&>(message);
    if (packet.get_int(input.flag_index) == 1) {
        return 0;
    }
    return LineFunc(22909, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E2F78
// IDA_NAME: _ZN23Dispatcher_InformNotice7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_InformNotice::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_InformNotice::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    InformNoticeData& output = static_cast<InformNoticeData&>(param);
    output.result = check_error(user, message);
    if (output.result > 0) {
        return 0;
    }
    if (output.result < 0) {
        return LineFunc(22924, kProcessSignature, output.result, 0);
    }

    const MSG_INFORM_NOTICE& input =
        static_cast<const MSG_INFORM_NOTICE&>(message);
    user->handleInformNoticeFlag(input.flag_index);
    return 0;
}

// IDA_EA: 0x81E3008
// IDA_NAME: _ZN23Dispatcher_InformNotice11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_InformNotice::check_error(CUser *,MSG_BASE &)
int Dispatcher_InformNotice::check_error(CUser* user, MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return 1;
    }
    const MSG_INFORM_NOTICE& input =
        static_cast<const MSG_INFORM_NOTICE&>(message);
    return input.flag_index <= 0x7fu ? 0 : -1;
}

// IDA_EA: 0x81E3056
// IDA_NAME: _ZN23Dispatcher_InformNotice4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_InformNotice::send(CUser *,ParamBase &)
void Dispatcher_InformNotice::send(CUser* user, ParamBase& param) {
    const InformNoticeData& output =
        static_cast<const InformNoticeData&>(param);
    if (output.result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INFORM_NOTICE,
            static_cast<unsigned char>(output.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_INFORM_NOTICE);
}
