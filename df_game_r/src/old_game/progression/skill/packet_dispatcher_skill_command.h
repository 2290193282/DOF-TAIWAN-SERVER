#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD333.  The old handler is a vptr-only derivative, but unlike the state-0
// handlers it owns the MSG_SKILL_COMMAND/stSkillCommandParam state-3 template
// specialization in its canonical C quest-skill translation unit.
class Dispatcher_Skill_Command_All_Default
    : public IPacketDispatcher<MSG_SKILL_COMMAND,
                               stSkillCommandParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85971B8
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultC2Ev
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::Dispatcher_Skill_Command_All_Default(void)
    Dispatcher_Skill_Command_All_Default();

    // IDA_EA: 0x8245550
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultD2Ev
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::~Dispatcher_Skill_Command_All_Default()
    // IDA_EA: 0x8245580
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_DefaultD0Ev
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::~Dispatcher_Skill_Command_All_Default()
    virtual ~Dispatcher_Skill_Command_All_Default();

    // IDA_EA: 0x81D2EEE
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D2EF8
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D2F34
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_All_Default4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Skill_Command_All_Default::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Skill_Command_All_Default) == 0x04u,
              "CMD333 must retain its old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_H_
