#include "old_game/player/core/achievement_trigger_dispatcher.h"

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/achievement_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_AchievementTrigger::read(PacketBuf&, MSG_BASE&)";

}  // namespace

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824F5C6
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824F716
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824F720
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x824F7B4
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::send(CUser *,ParamBase&)
// IDA_EA: 0x825BFE0
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C010
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8598D16
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_ACHIEVEMENT_TRIGGER9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,
                                 ParamBase,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x859786A
// IDA_NAME: _ZN29Dispatcher_AchievementTriggerC2Ev
// DEMANGLED: Dispatcher_AchievementTrigger::Dispatcher_AchievementTrigger(void)
Dispatcher_AchievementTrigger::Dispatcher_AchievementTrigger() {}

// IDA_EA: 0x8244BDE
// IDA_NAME: _ZN29Dispatcher_AchievementTriggerD2Ev
// DEMANGLED: Dispatcher_AchievementTrigger::~Dispatcher_AchievementTrigger()
// IDA_EA: 0x8244C0E
// IDA_NAME: _ZN29Dispatcher_AchievementTriggerD0Ev
// DEMANGLED: Dispatcher_AchievementTrigger::~Dispatcher_AchievementTrigger()
Dispatcher_AchievementTrigger::~Dispatcher_AchievementTrigger() {}

// IDA_EA: 0x81D7DFC
// IDA_NAME: _ZN29Dispatcher_AchievementTrigger4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AchievementTrigger::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AchievementTrigger::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    MSG_ACHIEVEMENT_TRIGGER& input =
        static_cast<MSG_ACHIEVEMENT_TRIGGER&>(message);
    if (packet.get_int(input.achievement_id) != 1) {
        return LineFunc(16997, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.first) != 1) {
        return LineFunc(16999, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.second) != 1) {
        return LineFunc(17001, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.third) == 1) {
        return 0;
    }
    return LineFunc(17003, kReadSignature, 0, 0);
}

// IDA_EA: 0x81D7F1E
// IDA_NAME: _ZN29Dispatcher_AchievementTrigger7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AchievementTrigger::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AchievementTrigger::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)param;

    if (user->get_state() > 2) {
        const MSG_ACHIEVEMENT_TRIGGER& input =
            static_cast<const MSG_ACHIEVEMENT_TRIGGER&>(message);
        CAchievement* const achievement = static_cast<CAchievement*>(
            user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(15)));
        achievement->setTrigger(input.achievement_id,
                                input.first,
                                input.second,
                                input.third);
    }
    return 0;
}
