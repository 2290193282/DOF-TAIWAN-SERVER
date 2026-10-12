#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_CUSTOMIZING_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_CUSTOMIZING_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD332.  The matching state-3 template family is already owned by the
// CMD333 provider; this class supplies only its real derived vtable/body.
class Dispatcher_Skill_Command_Customizing
    : public IPacketDispatcher<MSG_SKILL_COMMAND,
                               stSkillCommandParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859719C
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_CustomizingC2Ev
    // DEMANGLED: Dispatcher_Skill_Command_Customizing::Dispatcher_Skill_Command_Customizing(void)
    Dispatcher_Skill_Command_Customizing();

    // IDA_EA: 0x824559E / 0x82455CE
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_CustomizingD2Ev / D0Ev
    // DEMANGLED: Dispatcher_Skill_Command_Customizing::~Dispatcher_Skill_Command_Customizing()
    virtual ~Dispatcher_Skill_Command_Customizing();

    // IDA_EA: 0x81D29AC
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Skill_Command_Customizing::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D2B42
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Skill_Command_Customizing::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D2DA4
    // IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Skill_Command_Customizing::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Skill_Command_Customizing) == 0x04u,
              "CMD332 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SKILL_COMMAND_CUSTOMIZING_H_
