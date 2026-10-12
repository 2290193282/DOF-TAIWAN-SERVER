#include "old_game/economy/mail/packet_dispatcher_change_letter_stat.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// The template has one old-server vtable owner.  It remains unregistered: no
// PacketDispatcher table or lifecycle source is changed by this C provider.
// IDA_EA: 0x82558CA
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8255A1A
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8255A24
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CF1C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CF4C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D96C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x85986DA
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_CHANGE_LETTER_STAT23stChangeLetterStatParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_LETTER_STAT,stChangeLetterStatParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CHANGE_LETTER_STAT,
                                 stChangeLetterStatParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x82311F2
// IDA_NAME: _ZN23stChangeLetterStatParamC2Ev
// DEMANGLED: stChangeLetterStatParam::stChangeLetterStatParam(void)
stChangeLetterStatParam::stChangeLetterStatParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x824778C
// IDA_NAME: _ZN23stChangeLetterStatParamD2Ev
// DEMANGLED: stChangeLetterStatParam::~stChangeLetterStatParam()
// IDA_EA: 0x82477BC
// IDA_NAME: _ZN23stChangeLetterStatParamD0Ev
// DEMANGLED: stChangeLetterStatParam::~stChangeLetterStatParam()
stChangeLetterStatParam::~stChangeLetterStatParam() {}

// IDA_EA: 0x8596188
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStatC2Ev
// DEMANGLED: Dispatcher_ChangeLetterStat::Dispatcher_ChangeLetterStat(void)
Dispatcher_ChangeLetterStat::Dispatcher_ChangeLetterStat()
    : IPacketDispatcher<MSG_CHANGE_LETTER_STAT,
                        stChangeLetterStatParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245D8A
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStatD2Ev
// DEMANGLED: Dispatcher_ChangeLetterStat::~Dispatcher_ChangeLetterStat()
// IDA_EA: 0x8245DBA
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStatD0Ev
// DEMANGLED: Dispatcher_ChangeLetterStat::~Dispatcher_ChangeLetterStat()
Dispatcher_ChangeLetterStat::~Dispatcher_ChangeLetterStat() {}

// IDA_EA: 0x81CE152
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStat4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeLetterStat::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeLetterStat::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHANGE_LETTER_STAT& input =
        static_cast<MSG_CHANGE_LETTER_STAT&>(message);
    input.letter_id = 0;
    input.stat = 0;
    if (packet.get_int(input.letter_id) != 1) {
        return LineFunc(10034,
                        "virtual int Dispatcher_ChangeLetterStat::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_short(input.stat) == 1) {
        return 0;
    }
    return LineFunc(10036,
                    "virtual int Dispatcher_ChangeLetterStat::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81CE1FC
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStat7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeLetterStat::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeLetterStat::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    MSG_CHANGE_LETTER_STAT& input =
        static_cast<MSG_CHANGE_LETTER_STAT&>(message);
    stChangeLetterStatParam& result =
        static_cast<stChangeLetterStatParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result >= 0) {
        result.result = WongWork::CMailBoxHelper::ReqChangeLetterStat(
            user, input.letter_id, input.stat);
        return 0;
    }
    return LineFunc(
        10052,
        "virtual int Dispatcher_ChangeLetterStat::process(CUser*, MSG_BASE&, ParamBase&)",
        result.result,
        user->get_acc_id());
}

// IDA_EA: 0x81CE2BA
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStat11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeLetterStat::check_error(CUser *,MSG_BASE &)
int Dispatcher_ChangeLetterStat::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81CE304
// IDA_NAME: _ZN27Dispatcher_ChangeLetterStat4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ChangeLetterStat::send(CUser *,ParamBase &)
void Dispatcher_ChangeLetterStat::send(CUser* user, ParamBase& param) {
    const int result = static_cast<stChangeLetterStatParam&>(param).result;
    if (result != 0x7fffffff && result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHANGE_LETTER_STAT,
            static_cast<unsigned char>(result));
    }
}
