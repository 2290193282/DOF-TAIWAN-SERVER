#include "old_game/activity/schedule/packet_dispatcher_sr08_event_unclaimed.h"

#include "common/manager/global_tools.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"


// This concrete specialization is intentionally absent from the incomplete
// dispatcher root table.  The old 606-slot constructor must be restored as a
// whole before any individual handler can be registered.

// IDA_EA: 0x8249F62
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824A0BE
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824A0C8
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D5B8
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x825B3B0
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825B3E0
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x859835A
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_LOG_INGAME_ADVERTISEMENT24stLogIngameAdvertisementL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,stLogIngameAdvertisement,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,
                                 stLogIngameAdvertisement,
                                 static_cast<ch_state>(2)>;


// IDA_EA: 0x822E306
// IDA_NAME: _ZN28MSG_LOG_INGAME_ADVERTISEMENTC2Ev
// DEMANGLED: MSG_LOG_INGAME_ADVERTISEMENT::MSG_LOG_INGAME_ADVERTISEMENT(void)
MSG_LOG_INGAME_ADVERTISEMENT::MSG_LOG_INGAME_ADVERTISEMENT()
    : MSG_BASE() {
}


// IDA_EA: 0x8249EF8
// IDA_NAME: _ZN24stLogIngameAdvertisementC2Ev
// DEMANGLED: stLogIngameAdvertisement::stLogIngameAdvertisement(void)
stLogIngameAdvertisement::stLogIngameAdvertisement()
    : ParamBase() {
}

// IDA_EA: 0x8249F14
// IDA_NAME: _ZN24stLogIngameAdvertisementD2Ev
// DEMANGLED: stLogIngameAdvertisement::~stLogIngameAdvertisement()
// IDA_EA: 0x8249F44
// IDA_NAME: _ZN24stLogIngameAdvertisementD0Ev
// DEMANGLED: stLogIngameAdvertisement::~stLogIngameAdvertisement()
stLogIngameAdvertisement::~stLogIngameAdvertisement() {
}


// IDA_EA: 0x85958AC
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisementC2Ev
// DEMANGLED: Dispatcher_LogIngameAdvertisement::Dispatcher_LogIngameAdvertisement(void)
Dispatcher_LogIngameAdvertisement::Dispatcher_LogIngameAdvertisement()
    : IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,
                        stLogIngameAdvertisement,
                        static_cast<ch_state>(2)>() {
}

// IDA_EA: 0x8243E28
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisementD2Ev
// DEMANGLED: Dispatcher_LogIngameAdvertisement::~Dispatcher_LogIngameAdvertisement()
// IDA_EA: 0x8243E58
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisementD0Ev
// DEMANGLED: Dispatcher_LogIngameAdvertisement::~Dispatcher_LogIngameAdvertisement()
Dispatcher_LogIngameAdvertisement::~Dispatcher_LogIngameAdvertisement() {
}


// IDA_EA: 0x81E4CD0
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisement4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_LogIngameAdvertisement::read(PacketBuf &,MSG_BASE &)
int Dispatcher_LogIngameAdvertisement::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    MSG_LOG_INGAME_ADVERTISEMENT& input =
        static_cast<MSG_LOG_INGAME_ADVERTISEMENT&>(message);
    if (packet.get_byte(input.state) != 1) {
        return LineFunc(
            23919,
            "virtual int Dispatcher_LogIngameAdvertisement::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_short(input.view_code) != 1) {
        return LineFunc(
            23921,
            "virtual int Dispatcher_LogIngameAdvertisement::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_byte(input.type_code) != 1) {
        return LineFunc(
            23923,
            "virtual int Dispatcher_LogIngameAdvertisement::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(input.banner_id) == 1) {
        return 0;
    }
    return LineFunc(
        23925,
        "virtual int Dispatcher_LogIngameAdvertisement::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}


// IDA_EA: 0x81E4DF2
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisement7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_LogIngameAdvertisement::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_LogIngameAdvertisement::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    stLogIngameAdvertisement& output =
        static_cast<stLogIngameAdvertisement&>(param);
    output.result = check_error(user, message);
    if (output.result > 0) {
        return 0;
    }
    if (output.result < 0) {
        return LineFunc(
            23939,
            "virtual int Dispatcher_LogIngameAdvertisement::process(CUser*, MSG_BASE&, ParamBase&)",
            output.result,
            0);
    }

    MSG_LOG_INGAME_ADVERTISEMENT& input =
        static_cast<MSG_LOG_INGAME_ADVERTISEMENT&>(message);
    InGameADManagerInstance()->WriteLog(user,
                                        input.state,
                                        input.view_code,
                                        input.type_code,
                                        input.banner_id);
    return 0;
}


// IDA_EA: 0x81E4EC8
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisement11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_LogIngameAdvertisement::check_error(CUser *,MSG_BASE &)
int Dispatcher_LogIngameAdvertisement::check_error(CUser* user,
                                                    MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() > 1) {
        return 0;
    }
    return 0x7fffffff;
}


// IDA_EA: 0x81E4F04
// IDA_NAME: _ZN33Dispatcher_LogIngameAdvertisement4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_LogIngameAdvertisement::send(CUser *,ParamBase &)
void Dispatcher_LogIngameAdvertisement::send(CUser* user, ParamBase& param) {
    stLogIngameAdvertisement& output =
        static_cast<stLogIngameAdvertisement&>(param);
    if (output.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_LOG_INGAME_ADVERTISEMENT,
            static_cast<unsigned char>(output.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_LOG_INGAME_ADVERTISEMENT);
}
