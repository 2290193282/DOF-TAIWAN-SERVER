#include "old_game/player/core/inform_notice_2nd_dispatcher.h"

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/character_inform_notice.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_InformNotice_2nd::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_InformNotice_2nd::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x822E298
// IDA_NAME: _ZN21MSG_INFORM_NOTICE_2NDC2Ev
// DEMANGLED: MSG_INFORM_NOTICE_2ND::MSG_INFORM_NOTICE_2ND(void)
MSG_INFORM_NOTICE_2ND::MSG_INFORM_NOTICE_2ND()
    : MSG_BASE(), flag_index(128u) {}

// IDA_EA: 0x8231BF0
// IDA_NAME: _ZN19InformNoticeData2ndC2Ev
// DEMANGLED: InformNoticeData2nd::InformNoticeData2nd(void)
InformNoticeData2nd::InformNoticeData2nd() : ParamBase(), result(0) {}

// IDA_EA: 0x82467CA
// IDA_NAME: _ZN19InformNoticeData2ndD2Ev
// DEMANGLED: InformNoticeData2nd::~InformNoticeData2nd()
// IDA_EA: 0x82467FA
// IDA_NAME: _ZN19InformNoticeData2ndD0Ev
// DEMANGLED: InformNoticeData2nd::~InformNoticeData2nd()
InformNoticeData2nd::~InformNoticeData2nd() {}

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824AC80
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824ADDC
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824ADE6
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D5D6
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x825B584
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B5B4
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x859907A
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_INFORM_NOTICE_2ND19InformNoticeData2ndL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_INFORM_NOTICE_2ND,InformNoticeData2nd,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_INFORM_NOTICE_2ND,
                                 InformNoticeData2nd,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597D02
// IDA_NAME: _ZN27Dispatcher_InformNotice_2ndC2Ev
// DEMANGLED: Dispatcher_InformNotice_2nd::Dispatcher_InformNotice_2nd(void)
Dispatcher_InformNotice_2nd::Dispatcher_InformNotice_2nd() {}

// IDA_EA: 0x824404A
// IDA_NAME: _ZN27Dispatcher_InformNotice_2ndD2Ev
// DEMANGLED: Dispatcher_InformNotice_2nd::~Dispatcher_InformNotice_2nd()
// IDA_EA: 0x824407A
// IDA_NAME: _ZN27Dispatcher_InformNotice_2ndD0Ev
// DEMANGLED: Dispatcher_InformNotice_2nd::~Dispatcher_InformNotice_2nd()
Dispatcher_InformNotice_2nd::~Dispatcher_InformNotice_2nd() {}

// IDA_EA: 0x81E30A4
// IDA_NAME: _ZN27Dispatcher_InformNotice_2nd4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_InformNotice_2nd::read(PacketBuf &,MSG_BASE &)
int Dispatcher_InformNotice_2nd::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_INFORM_NOTICE_2ND& input =
        static_cast<MSG_INFORM_NOTICE_2ND&>(message);
    if (packet.get_int(input.flag_index) == 1) {
        return 0;
    }
    return LineFunc(22968, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E30FA
// IDA_NAME: _ZN27Dispatcher_InformNotice_2nd7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_InformNotice_2nd::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_InformNotice_2nd::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    MSG_INFORM_NOTICE_2ND& input =
        static_cast<MSG_INFORM_NOTICE_2ND&>(message);
    InformNoticeData2nd& output = static_cast<InformNoticeData2nd&>(param);
    output.result = check_error(user, message);
    if (output.result > 0) {
        return 0;
    }
    if (output.result < 0) {
        return LineFunc(22983,
                        kProcessSignature,
                        output.result,
                        0);
    }

    CCharacInformNotice* const notice = static_cast<CCharacInformNotice*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(19)));
    if (notice) {
        notice->handleInformNoticeFlag(input.flag_index);
    }
    return 0;
}

// IDA_EA: 0x81E31F4
// IDA_NAME: _ZN27Dispatcher_InformNotice_2nd4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_InformNotice_2nd::send(CUser *,ParamBase &)
void Dispatcher_InformNotice_2nd::send(CUser* user, ParamBase& param) {
    const InformNoticeData2nd& output =
        static_cast<const InformNoticeData2nd&>(param);
    if (output.result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INFORM_NOTICE,
            static_cast<unsigned char>(output.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_INFORM_NOTICE);
}

// IDA_EA: 0x81E31A6
// IDA_NAME: _ZN27Dispatcher_InformNotice_2nd11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_InformNotice_2nd::check_error(CUser *,MSG_BASE &)
int Dispatcher_InformNotice_2nd::check_error(CUser* user,
                                             MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return 1;
    }
    const MSG_INFORM_NOTICE_2ND& input =
        static_cast<const MSG_INFORM_NOTICE_2ND&>(message);
    return input.flag_index <= 0x7fu ? 0 : -1;
}
