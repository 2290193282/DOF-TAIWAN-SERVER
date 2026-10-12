#include "old_game/progression/skill/packet_dispatcher_skill_command.h"

#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// This C-owned explicit instantiation is the sole source/vtable owner for the
// old MSG_SKILL_COMMAND/stSkillCommandParam/state-3 template family.  The
// generic dispatcher infrastructure remains A-owned; neither it nor the root
// PacketDispatcher receives a registration change here.
// IDA_EA: 0x8252874
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82529DE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82529E8
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C7CC
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C7FC
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D882
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598996
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SKILL_COMMAND19stSkillCommandParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_COMMAND,stSkillCommandParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SKILL_COMMAND,
                                 stSkillCommandParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x85971B8
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultC2Ev
// DEMANGLED: Dispatcher_Skill_Command_All_Default::Dispatcher_Skill_Command_All_Default(void)
Dispatcher_Skill_Command_All_Default::Dispatcher_Skill_Command_All_Default()
    : IPacketDispatcher<MSG_SKILL_COMMAND,
                        stSkillCommandParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245550
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultD2Ev
// DEMANGLED: Dispatcher_Skill_Command_All_Default::~Dispatcher_Skill_Command_All_Default()
// IDA_EA: 0x8245580
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultD0Ev
// DEMANGLED: Dispatcher_Skill_Command_All_Default::~Dispatcher_Skill_Command_All_Default()
Dispatcher_Skill_Command_All_Default::~Dispatcher_Skill_Command_All_Default() {}

// IDA_EA: 0x81D2EEE
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Skill_Command_All_Default::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Skill_Command_All_Default::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D2EF8
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Skill_Command_All_Default::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Skill_Command_All_Default::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)message;
    stSkillCommandParam& result = static_cast<stSkillCommandParam&>(param);
    if (static_cast<uint8_t>(user->defaultSkillCommand()) != 1u) {
        result.result = 252;
    }
    return 0;
}

// IDA_EA: 0x81D2F34
// IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Skill_Command_All_Default::send(CUser *,ParamBase &)
void Dispatcher_Skill_Command_All_Default::send(CUser* user, ParamBase& param) {
    const stSkillCommandParam& result =
        static_cast<const stSkillCommandParam&>(param);
    if (result.result != 0) {
        PacketGuard packet;
        packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                          ENUM_CMDPACKET_SKILL_COMMAND_ALL_DEFAULT);
        packet.put_byte(0);
        packet.put_short(static_cast<int16_t>(result.result));
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SKILL_COMMAND_ALL_DEFAULT);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
}
